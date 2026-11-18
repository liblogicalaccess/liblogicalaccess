#include <logicalaccess/plugins/cards/desfire/desfireconfiguration.hpp>
#include <logicalaccess/plugins/cards/desfire/desfirecommands.hpp>
#include <logicalaccess/plugins/llacommon/logs.hpp>

namespace logicalaccess
{
namespace desfire
{
namespace configuration
{

namespace
{

// ----------------------------------------
// Validation helpers
// ----------------------------------------

bool isValidUIDLength(VCUIDLength value)
{
    switch (value)
    {
    case VCUIDLength::UID_4_BYTES:
    case VCUIDLength::UID_7_BYTES:
    case VCUIDLength::UID_10_BYTES: return true;

    case VCUIDLength::RFU:
    default: return false;
    }
}

bool isValidTransactionTimer(TransactionTimer value)
{
    switch (value)
    {
    case TransactionTimer::DISABLED:
    case TransactionTimer::ONE_TO_THREE_SECONDS:
    case TransactionTimer::TEN_TO_THIRTY_SECONDS:
    case TransactionTimer::HUNDRED_TO_THREE_HUNDRED_SECONDS: return true;

    default: return false;
    }
}

} // namespace

// ----------------------------------------
// PDCapabilityInformation
// ----------------------------------------

ByteVector PDCapabilityInformation::serialize() const
{
    EXCEPTION_ASSERT_WITH_LOG(isValidUIDLength(uidLength),
        LibLogicalAccessException, "Invalid VCUID length : RFU encoding is not supported.");

    const auto uidLengthValue = static_cast<std::uint8_t>(uidLength);
    const auto vcTypeValue = static_cast<std::uint8_t>(vcType);

    EXCEPTION_ASSERT_WITH_LOG(vcTypeValue <= 0x3F,
        LibLogicalAccessException, "Invalid VCTID : value exceeds six bits.");

    EXCEPTION_ASSERT_WITH_LOG(vcTypeValue != 0x00 && vcTypeValue != 0x08,
        LibLogicalAccessException, "Invalid VCTID : reserved or RFU value.");

    const std::uint8_t info = static_cast<std::uint8_t>(uidLengthValue | vcTypeValue);

    return {info, pdCap1_1, pdCap1_2};
}

// ----------------------------------------
// PDCap1.2
// ----------------------------------------

namespace pd_cap1_2
{

std::uint8_t Configuration::serialize() const
{
    std::uint8_t cardSizeValue = 0x00;

    switch (cardSize)
    {
    case CardSize::LESS_THAN_1KB:
    case CardSize::KB_1:
    case CardSize::KB_2:
    case CardSize::KB_4:
    case CardSize::KB_8:
    case CardSize::KB_16:
    case CardSize::KB_32: cardSizeValue = static_cast<std::uint8_t>(cardSize); break;

    case CardSize::RESERVED:
    default:
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Invalid PDCap1.2 CardSize : reserved value.");
    }

    std::uint8_t proximityValue = 0x00;

    switch (proximityCheck)
    {
    case ProximityCheck::NOT_SUPPORTED:
    case ProximityCheck::SUPPORTED:
    case ProximityCheck::MANDATORY:
        proximityValue = static_cast<std::uint8_t>(proximityCheck);
        break;

    case ProximityCheck::RESERVED:
    default:
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Invalid PDCap1.2 ProximityCheck : reserved value.");
    }

    EXCEPTION_ASSERT_WITH_LOG(commanderKeysetId <= 0x07,
        LibLogicalAccessException, "Invalid PDCap1.2 Commander Keyset ID : value must fit in three bits.");

    const auto commanderValue = static_cast<std::uint8_t>(commanderKeysetId << 5);

    return static_cast<std::uint8_t>(cardSizeValue | proximityValue | commanderValue);
}

} // namespace pd_cap1_2

// ----------------------------------------
// PDAuthenticationCapabilities
// ----------------------------------------

ByteVector PDAuthenticationCapabilities::serialize() const
{
    EXCEPTION_ASSERT_WITH_LOG(pdCap2_1 == 0x00,
        LibLogicalAccessException, "PDCap2.1 is reserved and must be zero.");

    EXCEPTION_ASSERT_WITH_LOG(pdCap2_3 == 0x00,
        LibLogicalAccessException, "PDCap2.3 is reserved and must be zero.");

    EXCEPTION_ASSERT_WITH_LOG(pdCap2_4 == 0x00,
        LibLogicalAccessException, "PDCap2.4 is reserved and must be zero.");

    EXCEPTION_ASSERT_WITH_LOG(isValidTransactionTimer(pdCap2_2),
        LibLogicalAccessException, "Invalid PDCap2.2 TransactionTimer value.");

    // PDCap2.5 and PDCap2.6 are service operator defined ! Their values are intentionally preserved without interpretation
    return {pdCap2_1, static_cast<std::uint8_t>(pdCap2_2), pdCap2_3, pdCap2_4, pdCap2_5, pdCap2_6};
}

// ----------------------------------------
// PCDAuthenticationCapabilities
// ----------------------------------------

void PCDAuthenticationCapabilities::validate() const
{
    for (std::size_t i = 0; i < CAPABILITY_COUNT; ++i)
    {
        EXCEPTION_ASSERT_WITH_LOG(bytes[i] == 0x00,
            LibLogicalAccessException, "PCD authentication capability bytes are currently reserved and must be zero.");
    }
}

ByteVector PCDAuthenticationCapabilities::serialize(std::size_t transmittedLength) const
{
    EXCEPTION_ASSERT_WITH_LOG(transmittedLength <= CAPABILITY_COUNT,
        LibLogicalAccessException, "Invalid PCD authentication capability transmission length.");

    validate();

    return ByteVector(bytes.begin(), bytes.begin() + transmittedLength);
}

// ----------------------------------------
// ProtocolOptions
// ----------------------------------------

namespace protocol_options
{

ByteVector Configuration::serialize() const
{
    EXCEPTION_ASSERT_WITH_LOG((protocolOptionsA & static_cast<std::uint8_t>(~VALID_MASK_A)) == 0x00,
        LibLogicalAccessException, "ProtocolOptionsA contains reserved bits.");

    EXCEPTION_ASSERT_WITH_LOG((protocolOptionsB & static_cast<std::uint8_t>(~VALID_MASK_B)) == 0x00,
        LibLogicalAccessException, "ProtocolOptionsB contains reserved bits.");

    return {protocolOptionsA, protocolOptionsB};
}

} // namespace protocol_options

} // namespace configuration

} // namespace desfire

} // namespace logicalaccess