#include <logicalaccess/plugins/cards/duox/duoxconfiguration.hpp>

#include <logicalaccess/plugins/cards/desfire/desfireconfiguration.hpp>
#include <logicalaccess/plugins/cards/desfire/desfirelocation.hpp>
#include <logicalaccess/plugins/llacommon/logs.hpp>
#include <logicalaccess/myexception.hpp>

#include <algorithm>
#include <cstdint>
#include <limits>

namespace logicalaccess
{

namespace duox
{

namespace configuration
{

namespace
{

// ----------------------------------------
// Internal validation helpers
// ----------------------------------------

/**
 * \brief Validate the serialized ATS configuration.
 *
 * TL must match the actual ATS length.
 * FSCI is encoded in the lower four bits of T0.
 */
void validateATS(const ByteVector &data)
{
    EXCEPTION_ASSERT_WITH_LOG(!data.empty() && data.size() <= ats::MAX_SIZE,
        LibLogicalAccessException, "ATS length must be between 1 and 20 bytes.");

    EXCEPTION_ASSERT_WITH_LOG(data[0] == data.size(),
        LibLogicalAccessException, "ATS TL does not match its actual length.");

    if (data.size() >= 2)
    {
        const std::uint8_t fsci = data[1] & 0x0F;

        EXCEPTION_ASSERT_WITH_LOG(fsci <= 8, LibLogicalAccessException, "Unsupported ATS FSCI.");
    }
}

/**
 * \brief Validate Secure Messaging configuration.
 * 
 * Optional fields are controlled by their corresponding bits.
 */
void validateSecureMessaging(const ByteVector &data)
{
    EXCEPTION_ASSERT_WITH_LOG(data.size() >= 2 && data.size() <= 4,
        LibLogicalAccessException, "Secure Messaging configuration requires between two and four bytes.");

    const std::uint8_t configB = data[1];

    // Bits 7 and 1-0 are reserved.
    EXCEPTION_ASSERT_WITH_LOG((configB & 0x83) == 0, LibLogicalAccessException, "SMConfigB contains reserved bits.");

    const bool hasECCCurve         = (configB & secure_messaging::ENABLE_ECC_SELECT) != 0;
    const bool hasUIDEncryptionKey = (configB & secure_messaging::ENABLE_FAST_UID) != 0;

    const std::size_t expectedSize = 2 + (hasECCCurve ? 1 : 0) + (hasUIDEncryptionKey ? 1 : 0);

    EXCEPTION_ASSERT_WITH_LOG(data.size() == expectedSize,
        LibLogicalAccessException, "Secure Messaging payload length does not match its enabled optional fields.");

    std::size_t offset = 2;

    if (hasECCCurve)
    {
        const std::uint8_t curve = data[offset++];

        EXCEPTION_ASSERT_WITH_LOG(curve == static_cast<std::uint8_t>(CurveID::NIST_P256) ||
                                  curve == static_cast<std::uint8_t>(CurveID::BRAINPOOL_P256R1),
            LibLogicalAccessException, "Unsupported Secure Messaging ECC curve.");
    }

    if (hasUIDEncryptionKey)
    {
        const std::uint8_t keyId = data[offset];

        EXCEPTION_ASSERT_WITH_LOG(keyId <= 0x0D, LibLogicalAccessException, "Invalid UID encryption key identifier.");
    }
}

/**
 * \brief Validate Crypto API Management configuration.
 */
void validateCryptoAPI(const ByteVector &data)
{
    EXCEPTION_ASSERT_WITH_LOG(data.size() == 2,
        LibLogicalAccessException, "Crypto API Management requires exactly two bytes.");

    EXCEPTION_ASSERT_WITH_LOG(data[0] == static_cast<std::uint8_t>(crypto_api::Support::DISABLED) ||
                              data[0] == static_cast<std::uint8_t>(crypto_api::Support::ECDSA_SIGN),
        LibLogicalAccessException, "Unsupported Crypto API functionality.");

    const std::uint8_t accessConfiguration = data[1];

    EXCEPTION_ASSERT_WITH_LOG((accessConfiguration & 0xC0) == 0,
        LibLogicalAccessException, "CryptoAuthAC contains reserved bits.");

    EXCEPTION_ASSERT_WITH_LOG((accessConfiguration & 0x30) != 0x30,
        LibLogicalAccessException, "Invalid CryptoAuthAC communication mode.");
}

} // namespace

// ----------------------------------------
// PICC Configuration (0x00)
// ----------------------------------------

ByteVector picc::PICCConfiguration::serialize() const
{
    std::uint8_t value = 0x00;

    if (disableFormat)
        value |= 0x01;

    if (enableRandomId)
    {
        value |= 0x02;
        value |= static_cast<std::uint8_t>(randomIdMode);
    }

    return {value};
}

ByteVector picc::ApplicationConfiguration::serialize() const
{
    return {static_cast<std::uint8_t>(disableFormat ? 0x01 : 0x00)};
}

// ----------------------------------------
// Default Keys (0x01)
// ----------------------------------------

ByteVector default_keys::Configuration::serialize() const
{
    EXCEPTION_ASSERT_WITH_LOG(keyData.size() == 24 || keyData.size() == 32,
        LibLogicalAccessException, "DefaultKeys requires 24 or 32 key bytes.");

    ByteVector result;
    result.reserve(keyData.size() + 1);

    result.insert(result.end(), keyData.begin(), keyData.end());
    result.push_back(keyVersion);

    return result;
}

// ----------------------------------------
// ATS (0x02)
// ----------------------------------------

void ats::Configuration::validate() const
{
    validateATS(bytes);
}

ByteVector ats::Configuration::serialize() const
{
    validate();
    return bytes;
}

// ----------------------------------------
// SAK (0x03)
// ----------------------------------------

ByteVector sak::Configuration::serialize() const
{
    return {sak1, sak2};
}

// ----------------------------------------
// Secure Messaging Configuration (0x04)
// ----------------------------------------

ByteVector secure_messaging::Configuration::serialize() const
{
    EXCEPTION_ASSERT_WITH_LOG((configB & 0x83) == 0, LibLogicalAccessException, "SMConfigB contains reserved bits.");

    EXCEPTION_ASSERT_WITH_LOG(!hasCurve || (configB & ENABLE_ECC_SELECT),
        LibLogicalAccessException, "Curve requires ECC-based ISOSelectFile authentication.");

    EXCEPTION_ASSERT_WITH_LOG(!hasUIDEncryptionKey || (configB & ENABLE_FAST_UID),
        LibLogicalAccessException, "UID encryption key requires fast UID retrieval.");
    
    EXCEPTION_ASSERT_WITH_LOG(!(hasCurve && hasUIDEncryptionKey),
        LibLogicalAccessException, "Both optional Secure Messaging extensions cannot be serialized together.");

    EXCEPTION_ASSERT_WITH_LOG(!hasUIDEncryptionKey || uidEncryptionKey <= 0x0D,
        LibLogicalAccessException, "Invalid UID encryption key identifier.");

    if (hasCurve)
    {
        EXCEPTION_ASSERT_WITH_LOG(curve == CurveID::NIST_P256 || curve == CurveID::BRAINPOOL_P256R1,
            LibLogicalAccessException, "Unsupported ECC curve.");
    }

    ByteVector result;
    result.reserve(2 + (hasCurve ? 1 : 0) + (hasUIDEncryptionKey ? 1 : 0));

    result.push_back(configA);
    result.push_back(configB);

    if (hasCurve)
        result.push_back(static_cast<std::uint8_t>(curve));

    if (hasUIDEncryptionKey)
        result.push_back(uidEncryptionKey);

    // Validate the final representation as a defensive consistency check.
    validateSecureMessaging(result);

    return result;
}

// ----------------------------------------
// Capability Data (0x05)
// ----------------------------------------

ByteVector capability::Configuration::serialize() const
{
    EXCEPTION_ASSERT_WITH_LOG((vctidOverride & 0xC0) == 0,
        LibLogicalAccessException, "VCTID override exceeds six bits.");

    EXCEPTION_ASSERT_WITH_LOG(vctidOverride == 0x00 || vctidOverride == 0x04 || vctidOverride == 0x09,
        LibLogicalAccessException, "Unsupported VCTID override.");

    const std::uint8_t timer = static_cast<std::uint8_t>(transactionTimer);

    EXCEPTION_ASSERT_WITH_LOG(timer <= 0x03, LibLogicalAccessException, "Invalid TransactionTimer.");

    return {0x00,  vctidOverride, 0x00, pdCap1_2, 0x00,
            timer, 0x00,          0x00, pdCap2_5, pdCap2_6};
}

// ----------------------------------------
// VC Installation Identifier (0x06)
// ----------------------------------------

ByteVector installation_id::Configuration::serialize() const
{
    EXCEPTION_ASSERT_WITH_LOG(!name.empty() && name.size() <= MAX_NAME_SIZE,
        LibLogicalAccessException, "Invalid VCIID/DFName length.");

    const bool isDelegated = target == Target::DELEGATED_APPLICATION;

    EXCEPTION_ASSERT_WITH_LOG(target == Target::PICC || target == Target::CONVENTIONAL_APPLICATION ||
                              target == Target::DELEGATED_APPLICATION,
        LibLogicalAccessException, "Invalid VCIID/DFName configuration target.");

    if (isDelegated)
    {
        EXCEPTION_ASSERT_WITH_LOG(damMac.size() == 8,
            LibLogicalAccessException, "Delegated application DAMMAC must contain exactly eight bytes.");
    }
    else
    {
        EXCEPTION_ASSERT_WITH_LOG(damMac.empty(),
            LibLogicalAccessException, "DAMMAC is only applicable to delegated applications.");
    }

    ByteVector result;
    result.reserve(isDelegated ? 25 : 17);

    result.push_back(static_cast<std::uint8_t>(name.size()));
    result.insert(result.end(), name.begin(), name.end());
    result.resize(17, 0x00); // Pad identifier to its fixed 16 byte field

    if (isDelegated)
        result.insert(result.end(), damMac.begin(), damMac.end());

    return result;
}

// ----------------------------------------
// ATQA (0x0C)
// ----------------------------------------

atqa::Configuration::Configuration(std::uint16_t atqa)
    : value(atqa)
{
}

ByteVector atqa::Configuration::serialize() const
{
    return {static_cast<std::uint8_t>(value & 0xFF),
            static_cast<std::uint8_t>((value >> 8) & 0xFF)};
}

atqa::Configuration atqa::Configuration::randomId4()
{
    return Configuration(0x0304);
}

atqa::Configuration atqa::Configuration::uid7()
{
    return Configuration(0x0344);
}

atqa::Configuration atqa::Configuration::uid10()
{
    return Configuration(0x0384);
}

// ----------------------------------------
// NFC Management (0x0F)
// ----------------------------------------

ByteVector nfc::Configuration::serialize() const
{
    EXCEPTION_ASSERT_WITH_LOG(support == 0x00, LibLogicalAccessException, "NFCSupport is RFU and must be zero.");

    EXCEPTION_ASSERT_WITH_LOG((protocolOptionsA & ~desfire::configuration::protocol_options::VALID_MASK_A) == 0,
        LibLogicalAccessException, "NFC ProtocolOptionsA contains reserved bits.");

    EXCEPTION_ASSERT_WITH_LOG((protocolOptionsB & ~desfire::configuration::protocol_options::VALID_MASK_B) == 0,
        LibLogicalAccessException, "NFC ProtocolOptionsB contains reserved bits.");

    return {support, protocolOptionsA, protocolOptionsB};
}

// ----------------------------------------
// I2C Management (0x10)
// ----------------------------------------

ByteVector i2c::Configuration::serialize() const
{
    EXCEPTION_ASSERT_WITH_LOG(support == 0x00, LibLogicalAccessException, "I2CSupport is RFU and must be zero.");

    EXCEPTION_ASSERT_WITH_LOG((protocolOptionsA & ~desfire::configuration::protocol_options::VALID_MASK_A) == 0,
        LibLogicalAccessException, "I2C ProtocolOptionsA contains reserved bits.");

    EXCEPTION_ASSERT_WITH_LOG((protocolOptionsB & ~desfire::configuration::protocol_options::VALID_MASK_B) == 0,
        LibLogicalAccessException, "I2C ProtocolOptionsB contains reserved bits.");

    return {support, address, protocolOptionsA, protocolOptionsB};
}

// ----------------------------------------
// GPIO Management (0x11)
// ----------------------------------------

ByteVector gpio::Configuration::serialize() const
{
    EXCEPTION_ASSERT_WITH_LOG(rawData.size() == 33,
        LibLogicalAccessException, "GPIO configuration must contain exactly 33 bytes.");

    // GPIO configuration is deliberately handled as raw data
    // The caller is responsible for supplying a correctly formatted 33 byte configuration

    return rawData;
}

// ----------------------------------------
// Crypto API Management (0x15)
// ----------------------------------------

ByteVector crypto_api::Configuration::serialize() const
{
    EXCEPTION_ASSERT_WITH_LOG(support == Support::DISABLED || support == Support::ECDSA_SIGN,
        LibLogicalAccessException, "Unsupported CryptoAPI support value.");

    EXCEPTION_ASSERT_WITH_LOG(commMode == CM_PLAIN || commMode == CM_MAC || commMode == CM_ENCRYPT,
        LibLogicalAccessException, "Invalid CryptoAPI communication mode.");

    EXCEPTION_ASSERT_WITH_LOG(accessCondition <= 0x0F,
        LibLogicalAccessException, "Invalid CryptoAuthAC access condition.");

    const std::uint8_t commModeValue = static_cast<std::uint8_t>(commMode);

    return {static_cast<std::uint8_t>(support), static_cast<std::uint8_t>(commModeValue | accessCondition)};
}

// ----------------------------------------
// Authentication Counter and Limit (0x16)
// ----------------------------------------

ByteVector authentication_limit::Configuration::serialize() const
{
    return {0x00, // Reserved
            static_cast<std::uint8_t>(enableEV2First ? 0x01 : 0x00),
            static_cast<std::uint8_t>(limit & 0xFF),
            static_cast<std::uint8_t>((limit >> 8) & 0xFF),
            static_cast<std::uint8_t>((limit >> 16) & 0xFF),
            static_cast<std::uint8_t>((limit >> 24) & 0xFF)};
}

// ----------------------------------------
// HALT and Wake-Up (0x17)
// ----------------------------------------

ByteVector halt_wakeup::Configuration::serialize() const
{
    EXCEPTION_ASSERT_WITH_LOG(rawData.size() == 4,
        LibLogicalAccessException, "HALT/Wake-Up configuration requires exactly four bytes.");

    // HALT/Wake-Up configuration is deliberately handled as raw data
    // The caller is responsible for supplying a correctly formatted four byte configuration

    return rawData;
}

// ----------------------------------------
// Lock Configuration (0xFF)
// ----------------------------------------

ByteVector locking::Configuration::serialize() const
{
    return ByteVector(lockMap.begin(), lockMap.end());
}

// ----------------------------------------
// SetConfiguration Payload Validation
// ----------------------------------------

void Payload::validate(Level selectedLevel, bool delegatedApplication) const
{
    EXCEPTION_ASSERT_WITH_LOG(!data.empty() && data.size() <= 33,
        LibLogicalAccessException, "SetConfiguration data must contain between 1 and 33 bytes.");

    const bool piccLevel = selectedLevel == Level::PICC;

    EXCEPTION_ASSERT_WITH_LOG(piccLevel || selectedLevel == Level::APPLICATION,
        LibLogicalAccessException, "Invalid SetConfiguration selection level.");

    EXCEPTION_ASSERT_WITH_LOG(!piccLevel || !delegatedApplication,
        LibLogicalAccessException, "Delegated application cannot be selected at PICC level.");

    switch (option)
    {
    case option::PICC_CONFIGURATION:
        EXCEPTION_ASSERT_WITH_LOG(data.size() == 1,
            LibLogicalAccessException, "PICC/Application configuration requires one byte.");
        break;

    case option::DEFAULT_KEYS:
        EXCEPTION_ASSERT_WITH_LOG(piccLevel, LibLogicalAccessException, "DefaultKeys is PICC-only.");

        EXCEPTION_ASSERT_WITH_LOG(data.size() == 25 || data.size() == 33,
            LibLogicalAccessException, "DefaultKeys requires 25 or 33 bytes.");
        break;

    case option::ATS:
        EXCEPTION_ASSERT_WITH_LOG(piccLevel, LibLogicalAccessException, "ATS is PICC-only.");

        EXCEPTION_ASSERT_WITH_LOG(data.size() >= 1 && data.size() <= ats::MAX_SIZE && data[0] == data.size(),
            LibLogicalAccessException, "Invalid ATS configuration.");

        if (data.size() >= 2)
            EXCEPTION_ASSERT_WITH_LOG((data[1] & 0x0F) <= 8, LibLogicalAccessException, "Unsupported ATS FSCI.");

        break;

    case option::SAK:
        EXCEPTION_ASSERT_WITH_LOG(piccLevel && data.size() == 2,
            LibLogicalAccessException, "SAK configuration is PICC-only and requires two bytes.");
        break;

    case option::SECURE_MESSAGING:
        EXCEPTION_ASSERT_WITH_LOG(data.size() == 2 || data.size() == 3,
            LibLogicalAccessException, "Secure Messaging requires two, three or four bytes.");
        break;

    case option::CAPABILITY_DATA:
        EXCEPTION_ASSERT_WITH_LOG(data.size() == 10,
            LibLogicalAccessException, "Capability Data requires ten bytes.");
        break;

    case option::VC_INSTALLATION_ID:
    {
        const std::size_t expectedSize = piccLevel || !delegatedApplication ? 17 : 25;

        EXCEPTION_ASSERT_WITH_LOG(data.size() == expectedSize,
            LibLogicalAccessException, "VCIID/DFName payload size does not match the selected configuration context.");
        break;
    }

    case option::ATQA:
        EXCEPTION_ASSERT_WITH_LOG(piccLevel && data.size() == 2,
            LibLogicalAccessException, "ATQA configuration is PICC-only and requires two bytes.");
        break;

    case option::NFC_MANAGEMENT:
        EXCEPTION_ASSERT_WITH_LOG(piccLevel && data.size() == 3,
            LibLogicalAccessException, "NFC Management is PICC-only and requires three bytes.");
        break;

    case option::I2C_MANAGEMENT:
        EXCEPTION_ASSERT_WITH_LOG(piccLevel && data.size() == 4,
            LibLogicalAccessException, "I2C Management is PICC-only and requires four bytes.");
        break;

    case option::GPIO_MANAGEMENT:
        EXCEPTION_ASSERT_WITH_LOG(piccLevel && data.size() == 33,
            LibLogicalAccessException, "GPIO Management is PICC-only and requires 33 bytes.");
        break;

    case option::CRYPTO_API_MANAGEMENT:
        EXCEPTION_ASSERT_WITH_LOG(data.size() == 2,
            LibLogicalAccessException, "CryptoAPI Management requires two bytes.");
        break;

    case option::AUTHENTICATION_LIMIT:
        EXCEPTION_ASSERT_WITH_LOG(data.size() == 6,
            LibLogicalAccessException, "Authentication Counter and Limit requires six bytes.");
        break;

    case option::HALT_WAKEUP:
        EXCEPTION_ASSERT_WITH_LOG(!piccLevel && data.size() == 4,
            LibLogicalAccessException, "HALT/Wake-Up is application-only and requires four bytes.");
        break;

    case option::LOCK_CONFIGURATION:
        EXCEPTION_ASSERT_WITH_LOG(!piccLevel && data.size() == 3,
            LibLogicalAccessException, "Lock Configuration is application-only and requires three bytes.");
        break;

    default:
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Unsupported, reserved or RFU SetConfiguration option.");
    }
}

} // namespace configuration

} // namespace duox

} // namespace logicalaccess