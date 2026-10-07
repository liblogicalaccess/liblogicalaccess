//
// Created by xaqq on 3/20/15.
//
#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include <logicalaccess/lla_core_api.hpp>
#include <logicalaccess/lla_fwd.hpp>

namespace logicalaccess
{
/**
 * \brief Helper representing a TLV (tag, length, value) data structure.
 *
 * The historical LibLogicalAccess TLV format uses :
 *       + The tag is one byte.
 *       + The length is one byte.
 *       + The value size is variable.
 *
 * A TLV's value can itself contain one or more TLVs.
 * 
 * This class also supports ASN.1 DER definite-length encoding which is used by X.509 v3 certificates
 * 
 * The class was originally designed for small TLV values and historically
 * stored the encoded length in sizeTag_ and sizeVector_
 * Those members and their public accessors are retained for backwards compatibility.
 */
class LLA_CORE_API TLV
{
  public:
    /**
     * \brief Construct an empty TLV with the specified tag.
     *
     * \param tag One-byte TLV tag.
     */
    explicit TLV(std::uint8_t tag);

    virtual ~TLV() = default;

    TLV(const TLV &)            = delete;
    TLV(TLV &&)                 = delete;
    TLV &operator=(const TLV &) = delete;
    TLV &operator=(TLV &&)      = delete;

    /**
     * \brief Return the TLV tag.
     */
    std::uint8_t tag() const noexcept;
    /**
     * \brief Set the TLV tag.
     */
    void tag(std::uint8_t t) noexcept;

    /**
     * \brief Return a copy of the "value" of this TLV.
     * If this TLV wraps another TLV, returns the `compute()` version of the sub TLV.
     */
    ByteVector value() const;
    
    /**
     * \brief Return the value as exactly one byte.
     * \throws LibLogicalAccessException if the value does not contain exactly one byte.
     */
    std::uint8_t value_u1() const;

    /**
     * \brief Set a boolean value.
     *
     * true is encoded as 0x01 and false as 0x00.
     */
    void value(bool v);

    /**
     * \brief Set a one-byte value.
     */
    void value(unsigned char v);

    /**
     * \brief Set a raw byte value.
     *
     * Any existing child TLVs are removed.
     */
    void value(const ByteVector &v);

    /**
     * \brief Set a single child TLV as the value.
     *
     * The supplied TLV is not copied.
     *
     * \param tlv Child TLV. Must not be nullptr.
     * 
     * \throws LibLogicalAccessException if tlv is nullptr
     */
    void value(TLVPtr tlv);

    /**
     * \brief Set a collection of TLVs as the value.
     *
     * The supplied TLVs are not copied. Any existing raw value and child TLVs are replaced.
     *
     * \param tlv Child TLVs.
     * 
     * \throws LibLogicalAccessException if a child pointer is nullptr
     */
    void value(std::vector<TLVPtr> tlv);

    /**
     * \brief Encode this TLV using the historical LibLogicalAccess format.
     *
     * The tag is encoded as one byte and the value length is encoded as one byte.
     * Therefore, values larger than 255 bytes cannot be encoded
     *
     * \throws LibLogicalAccessException if the value is larger than 255 bytes
     */
    ByteVector compute() const;

    /**
     * \brief Encode this TLV using canonical ASN.1 DER definite-length encoding.
     *
     * Values from 0 through 127 bytes use the DER short form. Values of 128 bytes or more use the DER long form
     *
     * \return Canonically encoded DER TLV.
     *
     * \throws LibLogicalAccessException if the value size cannot be represented safely
     */
    ByteVector compute_der() const;

    /**
     * \brief Return the first child having the specified tag.
     *
     * If explicit children have been assigned, those children are searched.
     * Otherwise the raw value is interpreted as a sequence of TLVs
     */
    TLVPtr get_child(std::uint8_t tag) const;

    /**
     * \brief Return the explicitly assigned child TLVs.
     */
    std::vector<TLVPtr> get_childs() const;

    /**
     * \brief Return the first encoded length byte
     *
     * For the legacy one-byte length representation, this is the value length
     *
     * For a DER long-form representation retained from parsing this is : 0x80 | the number of subsequent length bytes
     *
     * This API is retained for backwards compatibility
     */
    std::uint8_t getSizeTag() const noexcept;

    /**
     * \brief Set the legacy encoded size descriptor.
     *
     * This function is retained for backwards compatibility.
     *
     * New code should normally not need to call this method because
     * compute() and compute_der() derive the encoded length from value()
     */
    void setSizeTag(std::uint8_t tag) noexcept;

    /**
     * \brief Return the legacy encoded length vector.
     *
     * For short form, this contains the single length byte.
     *
     * For a long-form representation, this contains the length payload bytes, excluding the sizeTag_ descriptor byte
     */
    ByteVector getSizeVector() const;

    /**
     * \brief Set the legacy encoded length vector.
     */
    void setSizeVector(ByteVector size);

    /**
     * \brief Return the complete TLV using the stored length metadata.
     *
     * This function does not recompute or validate the length against value().
     * Returned representation may be invalid if the stored length metadata does not describe the current value
     */
    ByteVector getCompleteTLV() const;

    /**
     * \brief Find a child TLV by tag.
     *
     * \param tlvs Collection of TLVs to search.
     * \param tag Requested tag.
     * \param required If true, an exception is raised when no child exists.
     *
     * \return Matching TLV, or nullptr when required is false and no match exists.
     */
    static TLVPtr get_child(std::vector<TLVPtr> tlvs, std::uint8_t tag, bool required = true);

    /**
     * \brief Parse a sequence of TLVs.
     *
     * Parsing stops before an incomplete trailing TLV. In that situation
     * bytes_consumed identifies the amount of complete input consumed.
     *
     * The legacy format supports a one-byte value length and therefore accepts values from 0 through 255 bytes.
     *
     * \param bytes Input bytes.
     * \param bytes_consumed Number of bytes successfully consumed.
     */
    static std::vector<TLVPtr> parse_tlvs(const ByteVector &bytes, std::size_t &bytes_consumed);

    /**
     * \brief Parse a sequence of DER TLVs.
     *
     * Parsing stops before an incomplete trailing TLV. In that situation
     * bytes_consumed identifies the amount of complete input consumed.
     *
     * DER indefinite-length encoding is rejected. Non-canonical DER length encodings are rejected.
     *
     * \param bytes Input buffer.
     * \param bytes_consumed Number of bytes successfully consumed.
     *
     * \throws LibLogicalAccessException if a malformed or non-canonical DER length is encountered
     */
    static std::vector<TLVPtr> parse_tlvs_der(const ByteVector &bytes, std::size_t &bytes_consumed);

    /**
     * \brief Parse a sequence of TLVs.
     *
     * If strict is false, incomplete trailing input is tolerated for backwards compatibility.
     *
     * If strict is true, the entire input must contain complete TLVs.
     *
     * \param bytes Input bytes.
     * \param strict Require complete consumption of the input.
     *
     * \throws LibLogicalAccessException if strict parsing does not consume
     * the complete input or a malformed length is encountered
     */
    static std::vector<TLVPtr> parse_tlvs(const ByteVector &bytes, bool strict = false);

    /**
     * \brief Parse a sequence of DER TLVs.
     *
     * If strict is false, incomplete trailing input is tolerated.
     *
     * If strict is true, the entire input must contain complete DER TLVs.
     *
     * \param bytes Input buffer.
     * \param strict Require complete consumption of the input.
     *
     * \throws LibLogicalAccessException if malformed or non-canonical DER encoding is encountered,
     * or strict parsing does not consume the complete input
     */
    static std::vector<TLVPtr> parse_tlvs_der(const ByteVector &bytes, bool strict = false);

    /**
     * \brief Encode a sequence of TLVs as a concatenated byte vector.
     */
    static ByteVector value_tlvs(std::vector<TLVPtr> tlvs);

  private:
    /**
     * \brief Encode a length using canonical ASN.1 DER encoding.
     */
    static ByteVector encode_length(std::size_t length);

    /**
     * \brief Encode the value of this TLV
     *
     * If der is true, child TLVs are recursively encoded using DER.
     * Otherwise, children use the historical LibLogicalAccess encoding
     */
    ByteVector encode_value(bool der) const;

    /**
     * \brief Decode a DER definite-length field
     *
     * \param bytes Input buffer.
     * \param offset Current offset.
     * \param sizeTag Encoded first length byte.
     * \param sizeVector Encoded length payload bytes.
     * \param length Decoded value length.
     *
     * \return true if a complete length field was decoded
     *
     * \throws LibLogicalAccessException if the length is malformed, non-canonical, indefinite,
     * or cannot be represented by std::size_t
     */
    static bool decode_der_length(const ByteVector &bytes, std::size_t &offset, std::uint8_t &sizeTag,
        ByteVector &sizeVector, std::size_t &length);

    /**
     * \brief Compute the total encoded size of a collection of children.
     */
    static std::size_t encoded_children_size(const std::vector<TLVPtr> &tlvs);

    ByteVector sizeVector_;
    std::uint8_t tag_;
    std::uint8_t sizeTag_;
    ByteVector value_;            // Mutually exclusive with subTLVs_
    std::vector<TLVPtr> subTLVs_; // Mutually exclusive with value_
};

} // namespace logicalaccess
