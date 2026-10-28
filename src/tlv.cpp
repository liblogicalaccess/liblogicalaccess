//
// Created by xaqq on 3/20/15.
//

#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <utility>
#include <vector>

#include <logicalaccess/myexception.hpp>
#include <logicalaccess/tlv.hpp>

#include <logicalaccess/plugins/llacommon/logs.hpp>
#include <logicalaccess/plugins/llacommon/settings.hpp>


namespace logicalaccess
{

namespace
{
/**
 * \brief Throw a LibLogicalAccessException for invalid TLV state/input
 */
[[noreturn]] void throw_tlv_error(const char *message)
{
    THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, message);
}

/**
 * \brief Validate that a size addition cannot overflow std::size_t
 */
std::size_t checked_add(std::size_t lhs, std::size_t rhs)
{
    if (rhs > std::numeric_limits<std::size_t>::max() - lhs)
        throw_tlv_error("TLV encoded size overflows size_t.");
    return lhs + rhs;
}

/**
 * \brief Validate a child pointer
 */
void validate_child(const TLVPtr &child)
{
    if (!child)
        throw_tlv_error("TLV child cannot be null.");
}

/**
 * \brief Validate a collection of child pointers
 */
void validate_children(const std::vector<TLVPtr> &tlvs)
{
    for (const auto &tlv : tlvs)
        validate_child(tlv);
}

} // namespace

TLV::TLV(std::uint8_t tag)
    : sizeVector_{}
    , tag_(tag)
    , sizeTag_(0U)
    , value_{}
    , subTLVs_{}
{
}

std::uint8_t TLV::tag() const noexcept
{
    return tag_;
}

void TLV::tag(std::uint8_t t) noexcept
{
    tag_ = t;
}

ByteVector TLV::value() const
{
    return encode_value(false);
}

std::uint8_t TLV::value_u1() const
{
    const ByteVector v = value();
    if (v.size() != 1U)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "TLV value must be exactly one-byte long.");
    return v[0];
}

void TLV::value(bool v)
{
    value(static_cast<unsigned char>(v ? 0x01U : 0x00U));
}

void TLV::value(unsigned char v)
{
    value(ByteVector{v});
}

void TLV::value(const ByteVector &v)
{
    // Raw value and structured children are mutually exclusive
    subTLVs_.clear();
    value_ = v;

    // Stored length metadata no longer describes the value
    sizeTag_ = 0U;
    sizeVector_.clear();
}

void TLV::value(TLVPtr tlv)
{
    validate_child(tlv);

    value_.clear();
    subTLVs_.clear();
    subTLVs_.push_back(std::move(tlv));

    // Encoded size is derived from the children
    sizeTag_ = 0U;
    sizeVector_.clear();
}

void TLV::value(std::vector<TLVPtr> tlv)
{
    validate_children(tlv);

    value_.clear();
    subTLVs_ = std::move(tlv);

    sizeTag_ = 0U;
    sizeVector_.clear();
}

ByteVector TLV::compute() const
{
    const ByteVector encodedValue = value();

    if (encodedValue.size() > 0xFFU)
        throw_tlv_error("TLV value is too large for the legacy encoding.");

    ByteVector result;
    result.reserve(2U + encodedValue.size());

    result.push_back(tag_);
    result.push_back(static_cast<std::uint8_t>(encodedValue.size()));
    result.insert(result.end(), encodedValue.begin(), encodedValue.end());

    return result;
}

ByteVector TLV::compute_der() const
{
    const ByteVector encodedValue = encode_value(true);
    const ByteVector encodedLength = encode_length(encodedValue.size());
    const std::size_t headerSize = checked_add(static_cast<std::size_t>(1U), encodedLength.size());
    const std::size_t totalSize = checked_add(headerSize, encodedValue.size());

    ByteVector result;
    result.reserve(totalSize);

    result.push_back(tag_);
    result.insert(result.end(), encodedLength.begin(), encodedLength.end());
    result.insert(result.end(), encodedValue.begin(), encodedValue.end());

    return result;
}

ByteVector TLV::encode_value(bool der) const
{
    if (subTLVs_.empty())
        return value_;

    ByteVector result;

    for (const auto &tlv : subTLVs_)
    {
        validate_child(tlv);
        const ByteVector encoded = der ? tlv->compute_der() : tlv->compute();
        result.insert(result.end(), encoded.begin(), encoded.end());
    }

    return result;
}

ByteVector TLV::encode_length(std::size_t length)
{
    // DER definite-length encoding
    if (length < 0x80U)
        return ByteVector{static_cast<std::uint8_t>(length)};

    std::size_t value     = length;
    std::size_t byteCount = 0U;

    while (value != 0U)
    {
        ++byteCount;
        value >>= 8U;
    }

    if (byteCount > 0x7FU)
        throw_tlv_error("TLV length requires more than 127 length octets.");

    ByteVector result;
    result.reserve(byteCount + 1U);

    result.push_back(static_cast<std::uint8_t>(0x80U | static_cast<std::uint8_t>(byteCount)));
    for (std::size_t i = byteCount; i > 0U; --i)
    {
        const std::size_t shiftBits = (i - 1U) * 8U;
        result.push_back(static_cast<std::uint8_t>((length >> shiftBits) & 0xFFU));
    }

    return result;
}

bool TLV::decode_der_length(const ByteVector &bytes, std::size_t &offset, std::uint8_t &sizeTag,
    ByteVector &sizeVector, std::size_t &length)
{
    // Decode transactionally : if parsing cannot complete the caller's offset remains unchanged
    const std::size_t start = offset;

    sizeTag = 0U;
    sizeVector.clear();
    length = 0U;

    if (start >= bytes.size())
        return false;

    std::size_t current = start;
    const std::uint8_t firstLengthByte = bytes[current++];

    // DER short form
    if ((firstLengthByte & 0x80U) == 0U)
    {
        sizeTag = firstLengthByte;
        sizeVector.push_back(firstLengthByte);
        length = firstLengthByte;
        offset = current;
        return true;
    }

    // Long-form length (0x80 means indefinite length, which is forbidden by DER)
    const std::size_t lengthByteCount = static_cast<std::size_t>(firstLengthByte & 0x7FU);
    if (lengthByteCount == 0U)
        throw_tlv_error("Indefinite-length TLVs are not supported.");
    if (lengthByteCount > sizeof(std::size_t))
        throw_tlv_error("TLV length does not fit into size_t.");

    // Make sure the complete length field exists
    if (lengthByteCount > bytes.size() - current)
        return false;

    // Multi-byte length must not start with zero
    if (lengthByteCount > 1U && bytes[current] == 0U)
        throw_tlv_error("Non-canonical DER length encoding.");

    sizeTag = firstLengthByte;
    sizeVector.reserve(lengthByteCount);

    for (std::size_t i = 0U; i < lengthByteCount; ++i)
    {
        const std::uint8_t byte = bytes[current++];
        if (length > (std::numeric_limits<std::size_t>::max() >> 8U))
            throw_tlv_error("TLV length overflows size_t.");
        length = (length << 8U) | static_cast<std::size_t>(byte);
        sizeVector.push_back(byte);
    }

    if (length < 0x80U)
        throw_tlv_error("Non-canonical DER length encoding.");

    offset = current;
    return true;
}

std::size_t TLV::encoded_children_size(const std::vector<TLVPtr> &tlvs)
{
    std::size_t total = 0U;

    for (const auto &tlv : tlvs)
    {
        validate_child(tlv);
        const ByteVector encoded = tlv->compute();
        total = checked_add(total, encoded.size());
    }

    return total;
}

std::vector<TLVPtr> TLV::parse_tlvs(const ByteVector &bytes, std::size_t &bytes_consumed)
{
    std::vector<TLVPtr> tlvs;
    bytes_consumed = 0U;

    while (bytes_consumed < bytes.size())
    {
        const std::size_t start = bytes_consumed;

        // TAG
        const std::uint8_t tag = bytes[bytes_consumed++];

        // LENGTH
        // Tag without a following length byte is an incomplete TLV
        if (bytes_consumed >= bytes.size())
        {
            bytes_consumed = start;
            break;
        }

        const std::uint8_t valueLengthByte = bytes[bytes_consumed++];
        const std::size_t valueLength      = static_cast<std::size_t>(valueLengthByte);

        // Value
        if (valueLength > bytes.size() - bytes_consumed)
        {
            bytes_consumed = start;
            break;
        }

        auto tlv = std::make_shared<TLV>(tag);
        const auto valueBegin = bytes.begin() + static_cast<std::ptrdiff_t>(bytes_consumed);
        const auto valueEnd = valueBegin + static_cast<std::ptrdiff_t>(valueLength);
        ByteVector value(valueBegin, valueEnd);

        bytes_consumed += valueLength;

        tlv->value(value);
        tlv->setSizeTag(valueLengthByte);
        tlv->setSizeVector(ByteVector{valueLengthByte});

        tlvs.push_back(std::move(tlv));
    }
    return tlvs;
}

std::vector<TLVPtr> TLV::parse_tlvs_der(const ByteVector &bytes, std::size_t &bytes_consumed)
{
    std::vector<TLVPtr> tlvs;
    bytes_consumed = 0U;

    while (bytes_consumed < bytes.size())
    {
        const std::size_t start = bytes_consumed;

        // TAG
        const std::uint8_t tag = bytes[bytes_consumed++];

        // LENGTH
        std::uint8_t sizeTag = 0U;
        ByteVector sizeVector;
        std::size_t valueLength = 0U;

        if (!decode_der_length(bytes, bytes_consumed, sizeTag, sizeVector, valueLength))
        {
            // Incomplete LENGTH field
            bytes_consumed = start;
            break;
        }

        // VALUE
        // Note : bytes_consumed + valueLength > bytes.size() could overflow
        if (valueLength > bytes.size() - bytes_consumed)
        {
            // Incomplete TLV
            bytes_consumed = start;
            break;
        }

        auto tlv = std::make_shared<TLV>(tag);

        const auto valueBegin = bytes.begin() + static_cast<std::ptrdiff_t>(bytes_consumed);
        const auto valueEnd = valueBegin + static_cast<std::ptrdiff_t>(valueLength);

        ByteVector value(valueBegin, valueEnd);

        bytes_consumed += valueLength;

        tlv->value(value);
        tlv->setSizeTag(sizeTag);
        tlv->setSizeVector(std::move(sizeVector));

        tlvs.push_back(std::move(tlv));
    }

    return tlvs;
}

std::uint8_t TLV::getSizeTag() const noexcept
{
    return sizeTag_;
}

void TLV::setSizeTag(std::uint8_t tag) noexcept
{
    sizeTag_ = tag;
}

ByteVector TLV::getSizeVector() const
{
    return sizeVector_;
}

void TLV::setSizeVector(ByteVector size)
{
    if (size.size() == 1U)
        sizeTag_ = size[0];
    sizeVector_ = std::move(size);
}

ByteVector TLV::getCompleteTLV() const
{
    const std::size_t encodedSizeVector = sizeVector_.size() > 1U ? sizeVector_.size() : 0U;
    const std::size_t size = checked_add(checked_add(2U, encodedSizeVector), value_.size());

    ByteVector result;
    result.reserve(size);

    result.push_back(tag_);
    result.push_back(sizeTag_);
    if (sizeVector_.size() > 1U)
        result.insert(result.end(), sizeVector_.begin(), sizeVector_.end());
    result.insert(result.end(), value_.begin(), value_.end());

    return result;
}

std::vector<TLVPtr> TLV::parse_tlvs(const ByteVector &bytes, bool strict)
{
    std::size_t consumed = 0U;
    auto tlvs = parse_tlvs(bytes, consumed);

    if (strict && consumed != bytes.size())
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "TLV parsing did not reach the end of the buffer.");

    return tlvs;
}

std::vector<TLVPtr> TLV::parse_tlvs_der(const ByteVector &bytes, bool strict)
{
    std::size_t consumed = 0U;
    auto tlvs = parse_tlvs_der(bytes, consumed);

    if (strict && consumed != bytes.size())
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "DER TLV parsing did not reach the end of the buffer.");

    return tlvs;
}

// Note : this function
ByteVector TLV::value_tlvs(std::vector<TLVPtr> tlvs)
{
    validate_children(tlvs);

    const std::size_t totalSize = encoded_children_size(tlvs);

    ByteVector result;
    result.reserve(totalSize);

    for (const auto &tlv : tlvs)
    {
        const ByteVector encoded = tlv->compute();
        result.insert(result.end(), encoded.begin(), encoded.end());
    }

    return result;
}

TLVPtr TLV::get_child(std::uint8_t tag) const
{
    if (!subTLVs_.empty())
        return get_child(subTLVs_, tag, true);

    const auto tlvs = parse_tlvs(value_);
    return get_child(tlvs, tag, true);
}

std::vector<TLVPtr> TLV::get_childs() const
{
    return subTLVs_;
}

TLVPtr TLV::get_child(std::vector<TLVPtr> tlvs, std::uint8_t tag, bool required)
{
    for (const auto &tlv : tlvs)
    {
        validate_child(tlv);
        if (tlv->tag() == tag)
            return tlv;
    }

    if (required)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Cannot find expected child TLV.");

    return nullptr;
}
}
