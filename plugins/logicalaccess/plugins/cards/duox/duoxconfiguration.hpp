#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include <logicalaccess/lla_fwd.hpp>

#include <logicalaccess/plugins/cards/desfire/desfireconfiguration.hpp>
#include <logicalaccess/plugins/cards/desfire/desfirelocation.hpp>
#include <logicalaccess/plugins/cards/desfire/desfireev1commands.hpp>

#include <logicalaccess/plugins/cards/duox/duoxecc.hpp>

#include <logicalaccess/plugins/cards/desfire/lla_cards_desfire_api.hpp>

namespace logicalaccess
{

namespace duox
{

namespace configuration
{

// ----------------------------------------
// SetConfiguration command
// ----------------------------------------

/**
 * \brief Configuration selection level.
 *
 * Determines whether a configuration applies to the PICC or to the currently selected application.
 */
enum class Level
{
    PICC,
    APPLICATION
};

/**
 * \brief SetConfiguration option identifiers.
 *
 * These values identify the configuration data structure carried by the SetConfiguration command.
 *
 * Options marked PICC-only must be considered RFU at application level.
 */
namespace option
{

constexpr std::uint8_t PICC_CONFIGURATION    = 0x00;
constexpr std::uint8_t DEFAULT_KEYS          = 0x01;
constexpr std::uint8_t ATS                   = 0x02;
constexpr std::uint8_t SAK                   = 0x03;
constexpr std::uint8_t SECURE_MESSAGING      = 0x04;
constexpr std::uint8_t CAPABILITY_DATA       = 0x05;
constexpr std::uint8_t VC_INSTALLATION_ID    = 0x06;
constexpr std::uint8_t ATQA                  = 0x0C;
constexpr std::uint8_t NFC_MANAGEMENT        = 0x0F;
constexpr std::uint8_t I2C_MANAGEMENT        = 0x10;
constexpr std::uint8_t GPIO_MANAGEMENT       = 0x11;
constexpr std::uint8_t CRYPTO_API_MANAGEMENT = 0x15;
constexpr std::uint8_t AUTHENTICATION_LIMIT  = 0x16;
constexpr std::uint8_t HALT_WAKEUP           = 0x17;
constexpr std::uint8_t LOCK_CONFIGURATION    = 0xFF;

} // namespace option

// ----------------------------------------
// PICC and Application Configuration (0x00)
// ----------------------------------------

namespace picc
{

/**
 * \brief Random ID configuration mode.
 *
 * Defines the UID0 value used when Random ID is enabled
 */
enum class RandomIdMode : std::uint8_t
{
    ISO_COMPLIANT  = 0x00,
    LEGACY_DESFIRE = 0x20
};

/**
 * \brief PICC configuration.
 *
 * Serialized as one byte.
 * Random ID cannot be disabled once enabled.
 */
struct LLA_CARDS_DESFIRE_API PICCConfiguration
{
    /**
     * \brief Disable the Format command at PICC level.
     *
     * This setting is irreversible once enabled.
     */
    bool disableFormat = false;

    /**
     * \brief Enable Random ID.
     *
     * Random ID is disabled by default and cannot subsequently be disabled.
     */
    bool enableRandomId = false;

    /**
     * \brief Random ID UID0 configuration.
     *
     * Only relevant when enableRandomId is true.
     */
    RandomIdMode randomIdMode = RandomIdMode::ISO_COMPLIANT;

    /**
     * \brief Validate and serialize the PICC configuration.
     *
     * \return One byte PICC configuration.
     */
    ByteVector serialize() const;
};

/**
 * \brief Application configuration.
 *
 * Serialized as one byte.
 * This configuration is applicable to delegated applications.
 */
struct LLA_CARDS_DESFIRE_API ApplicationConfiguration
{
    /**
     * \brief Disable the Format command for the selected application.
     */
    bool disableFormat = false;

    /**
     * \brief Serialize the application configuration.
     *
     * \return One byte application configuration.
     */
    ByteVector serialize() const;
};

} // namespace picc

// ----------------------------------------
// Default Keys Update (0x01)
// ----------------------------------------

namespace default_keys
{

/**
 * \brief Default PICC application key configuration.
 *
 * Contains the default key material followed by its key version.
 *
 * The key material must contain either :
 * - 24 bytes for DESFire-compatible key material.
 * - 32 bytes for AES-256 key material.
 *
 * The first 16 bytes are used for AES-128 keys.
 */
struct LLA_CARDS_DESFIRE_API Configuration
{
    /**
     * \brief Default key material.
     *
     * Valid sizes are 24 or 32 bytes.
     */
    ByteVector keyData;

    /**
     * \brief Default key version.
     */
    std::uint8_t keyVersion = 0x00;

    /**
     * \brief Validate and serialize the default key configuration.
     *
     * \return Key material followed by the key version.
     */
    ByteVector serialize() const;
};

} // namespace default_keys

// ----------------------------------------
// ATS Update (0x02)
// ----------------------------------------

namespace ats
{

/**
 * \brief Maximum supported ATS length.
 */
constexpr std::size_t MAX_SIZE = 20;

/**
 * \brief Frame Size for proximity cards (FSCI).
 *
 * Encodes the maximum frame size supported by the PICC.
 */
enum class FSCI : std::uint8_t
{
    FSC_16_BYTES  = 0x00,
    FSC_24_BYTES  = 0x01,
    FSC_32_BYTES  = 0x02,
    FSC_40_BYTES  = 0x03,
    FSC_48_BYTES  = 0x04,
    FSC_64_BYTES  = 0x05,
    FSC_96_BYTES  = 0x06,
    FSC_128_BYTES = 0x07,
    FSC_256_BYTES = 0x08
};

/**
 * \brief Update the FSCI field of an ATS T0 byte.
 *
 * Preserves the upper four bits of T0.
 *
 * \param t0 Existing ATS T0 byte.
 * \param fsci New FSCI value.
 * \return Updated T0 byte.
 */
constexpr std::uint8_t updateFSCI(std::uint8_t t0, FSCI fsci)
{
    return static_cast<std::uint8_t>((t0 & 0xF0) | static_cast<std::uint8_t>(fsci));
}

/**
 * \brief User-defined ATS configuration.
 */
struct LLA_CARDS_DESFIRE_API Configuration
{
    /**
     * \brief Complete ATS byte sequence.
     *
     * Includes TL, T0 and all optional ATS interface bytes.
     */
    ByteVector bytes;

    /**
     * \brief Validate the ATS configuration.
     */
    void validate() const;

    /**
     * \brief Validate and serialize the ATS configuration.
     *
     * \return Complete ATS byte sequence.
     */
    ByteVector serialize() const;
};

/**
 * \brief Default DUOX ATS.
 */
constexpr std::array<std::uint8_t, 6> DEFAULT_ATS = {0x06, 0x78, 0x77, 0x81, 0x02, 0x80};

} // namespace ats

// ----------------------------------------
// SAK Update (0x03)
// ----------------------------------------

namespace sak
{

/**
 * \brief User-defined SAK configuration.
 */
struct LLA_CARDS_DESFIRE_API Configuration
{
    /**
     * \brief First SAK byte.
     */
    std::uint8_t sak1 = 0x24;

    /**
     * \brief Second SAK byte.
     */
    std::uint8_t sak2 = 0x20;

    /**
     * \brief Serialize the SAK configuration.
     *
     * \return Two byte SAK configuration.
     */
    ByteVector serialize() const;
};

} // namespace sak

// ----------------------------------------
// ATQA Update (0x0C)
// ----------------------------------------

namespace atqa
{

/**
 * \brief ATQA configuration.
 *
 * The ATQA value is represented as a 16-bit integer.
 *
 * Default values :
 * - Seven byte UID : 0x0344 (transmitted 0x44 0x03).
 * - Four byte Random ID : 0x0304 (transmitted 0x04 0x03).
 * - Ten byte UID : 0x0384 (transmitted 0x84 0x03).
 */
struct LLA_CARDS_DESFIRE_API Configuration
{
    /**
     * \brief ATQA value.
     */
    std::uint16_t value = 0x0344;

    /**
     * \brief Construct an ATQA configuration.
     */
    Configuration() = default;

    /**
     * \brief Construct an ATQA configuration.
     *
     * \param atqa ATQA value.
     */
    explicit Configuration(std::uint16_t atqa);

    /**
     * \brief Serialize the ATQA value.
     *
     * \return Two byte ATQA value, least significant byte first.
     */
    ByteVector serialize() const;

    /**
     * \brief Create the ATQA configuration for a four byte Random ID.
     *
     * \return ATQA value 0x0304.
     */
    static Configuration randomId4();

    /**
     * \brief Create the ATQA configuration for a seven byte UID.
     *
     * \return ATQA value 0x0344.
     */
    static Configuration uid7();

    /**
     * \brief Create the ATQA configuration for a ten byte UID.
     *
     * \return ATQA value 0x0384.
     */
    static Configuration uid10();
};

} // namespace atqa

// ----------------------------------------
// Secure Messaging Configuration (0x04)
// ----------------------------------------

namespace secure_messaging
{

constexpr std::uint8_t ENABLE_FAST_UID     = 0x40;
constexpr std::uint8_t ENABLE_FREE_I2C     = 0x20;
constexpr std::uint8_t ENABLE_EV_CHARGING  = 0x10;
constexpr std::uint8_t ENABLE_ECC_SELECT   = 0x08;
constexpr std::uint8_t ENABLE_EV2_CHAINING = 0x04;

/**
 * \brief Secure Messaging configuration.
 *
 * Serialized as :
 * - SMConfigA.
 * - SMConfigB.
 * - Optional CurveID when ECC-based authentication is enabled.
 * - Optional AppUIDENCKey when fast UID retrieval is enabled.
 *
 * CurveID and AppUIDENCKey are mutually exclusive according to the configuration format.
 */
struct LLA_CARDS_DESFIRE_API Configuration
{
    /**
     * \brief Secure Messaging configuration byte A.
     *
     * Its bit definitions are not specified in the supplied DUOX SetConfiguration options list.
     */
    std::uint8_t configA = 0x00;

    /**
     * \brief Secure Messaging configuration byte B.
     */
    std::uint8_t configB = 0x00;

    /**
     * \brief Indicates whether CurveID is present.
     */
    bool hasCurve = false;

    /**
     * \brief Selected ECC curve.
     */
    CurveID curve = CurveID::NIST_P256;

    /**
     * \brief Indicates whether AppUIDENCKey is present.
     */
    bool hasUIDEncryptionKey = false;

    /**
     * \brief Key identifier used for fast confidential UID retrieval.
     *
     * Valid values depend on the number of application keys and must be in the range 0x00-0x0D.
     */
    std::uint8_t uidEncryptionKey = 0x00;

    /**
     * \brief Validate and serialize the Secure Messaging configuration.
     *
     * \return Serialized Secure Messaging configuration.
     */
    ByteVector serialize() const;
};

} // namespace secure_messaging

// ----------------------------------------
// Capability Data (0x05)
// ----------------------------------------

namespace capability
{

/**
 * \brief Capability Data configuration.
 *
 * Configures the PD capabilities exchanged during virtual card selection and symmetric authentication.
 * PDCap1.2 and TransactionTimer reuse the types defined in desfire::configuration.
 */
struct LLA_CARDS_DESFIRE_API Configuration
{
    /**
     * \brief Virtual Card Type Identifier override.
     */
    std::uint8_t vctidOverride = 0x00;

    /**
     * \brief Service operator defined PDCap1.2.
     */
    std::uint8_t pdCap1_2 = 0x00;

    /**
     * \brief Transaction Timer configuration (PDCap2.2).
     */
    desfire::configuration::TransactionTimer transactionTimer = desfire::configuration::TransactionTimer::DISABLED;

    /**
     * \brief Service operator defined PDCap2.5.
     */
    std::uint8_t pdCap2_5 = 0x00;

    /**
     * \brief Service operator defined PDCap2.6.
     */
    std::uint8_t pdCap2_6 = 0x00;

    /**
     * \brief Validate and serialize Capability Data.
     *
     * \return Ten byte Capability Data representation.
     */
    ByteVector serialize() const;
};

} // namespace capability

// ----------------------------------------
// VCIID / ISO DFName (0x06)
// ----------------------------------------

namespace installation_id
{

/**
 * \brief Maximum permitted VCIID or ISO DFName length.
 */
constexpr std::size_t MAX_NAME_SIZE = 16;

/**
 * \brief Installation identifier target.
 *
 * Distinguishes the three supported payload contexts.
 */
enum class Target : std::uint8_t
{
    PICC,
    CONVENTIONAL_APPLICATION,
    DELEGATED_APPLICATION
};

/**
 * \brief VC Installation Identifier or application ISO DFName configuration.
 *
 * At PICC level, this option configures the VCIID.
 *
 * At application level, this option configures the ISO DFName.
 * 
 * The resulting payload is therefore either 17 or 25 bytes.
 */
struct LLA_CARDS_DESFIRE_API Configuration
{
    /**
     * \brief Indicates whether the target is PICC or application.
     */
    Target target = Target::PICC;

    /**
     * \brief VCIID or ISO DFName.
     *
     * Valid length : 1-16 bytes.
     */
    ByteVector name;

    /**
     * \brief Delegated Application MAC.
     *
     * Must contain exactly eight bytes for delegated applications.
     * Must be empty for conventional applications and PICC configuration.
     */
    ByteVector damMac;

    /**
     * \brief Validate and serialize the installation identifier.
     *
     * \return Serialized VCIID or ISO DFName configuration.
     */
    ByteVector serialize() const;
};

} // namespace installation_id

// ----------------------------------------
// NFC Management (0x0F)
// ----------------------------------------

namespace nfc
{

/**
 * \brief NFC Management configuration.
 *
 * Configures the cryptographic protocols supported over NFC.
 *
 * NFCSupport is currently RFU and must be zero.
 *
 * ProtocolOptions uses the definitions from desfire::configuration::protocol_options.
 */
struct LLA_CARDS_DESFIRE_API Configuration
{
    /**
     * \brief NFC support configuration.
     *
     * Currently RFU. Must be zero.
     */
    std::uint8_t support = 0x00;

    /**
     * \brief ProtocolOptions byte A.
     */
    std::uint8_t protocolOptionsA = 0x0F;

    /**
     * \brief ProtocolOptions byte B.
     *
     * All bits are currently RFU and must be zero.
     */
    std::uint8_t protocolOptionsB = 0x00;

    /**
     * \brief Validate and serialize NFC Management configuration.
     *
     * \return Three byte NFC Management configuration.
     */
    ByteVector serialize() const;
};

} // namespace nfc

// ----------------------------------------
// I2C Management (0x10)
// ----------------------------------------

namespace i2c
{

/**
 * \brief I2C Management configuration.
 *
 * Configures the I2C target address and the cryptographic protocols supported over I2C.
 *
 * I2CSupport is currently RFU and must be zero.
 */
struct LLA_CARDS_DESFIRE_API Configuration
{
    /**
     * \brief I2C support configuration.
     *
     * Currently RFU. Must be zero.
     */
    std::uint8_t support = 0x00;

    /**
     * \brief I2C target address.
     *
     * Default value is 0x20.
     */
    std::uint8_t address = 0x20;

    /**
     * \brief ProtocolOptions byte A.
     */
    std::uint8_t protocolOptionsA = 0x0F;

    /**
     * \brief ProtocolOptions byte B.
     *
     * All bits are currently RFU and must be zero.
     */
    std::uint8_t protocolOptionsB = 0x00;

    /**
     * \brief Validate and serialize I2C Management configuration.
     *
     * \return Four byte I2C Management configuration.
     */
    ByteVector serialize() const;
};

} // namespace i2c

// ----------------------------------------
// GPIO Management (0x11)
// ----------------------------------------

namespace gpio
{

/**
 * \brief GPIO Management configuration.
 *
 * GPIO Management requires a 33 byte configuration.
 *
 * GPIO management is deliberately not interpreted by this library.
 * The caller is responsible for supplying a correctly formatted configuration according to the card specification.
 */
struct LLA_CARDS_DESFIRE_API Configuration
{
    /**
     * \brief Raw GPIO configuration.
     *
     * Must contain exactly 33 bytes.
     */
    ByteVector rawData;

    /**
     * \brief Validate and serialize GPIO Management configuration.
     *
     * \return The supplied 33 byte GPIO configuration.
     */
    ByteVector serialize() const;
};

} // namespace gpio

// ----------------------------------------
// Crypto API Management (0x15)
// ----------------------------------------

namespace crypto_api
{

/**
 * \brief Supported Crypto API functionality.
 */
enum class Support : std::uint8_t
{
    DISABLED   = 0x00,
    ECDSA_SIGN = 0x02
};

/**
 * \brief Crypto API Management configuration.
 *
 * Configures Crypto API support and the access conditions for the CryptoRequest command.
 */
struct LLA_CARDS_DESFIRE_API Configuration
{
    /**
     * \brief Crypto API support.
     */
    Support support = Support::DISABLED;

    /**
     * \brief Communication mode.
     *
     * Default is CommMode.MAC.
     */
    EncryptionMode commMode = CM_MAC;

    /**
     * \brief CryptoRequest access condition.
     *
     * Valid range : 0x00-0x0F.
     */
    std::uint8_t accessCondition = 0x00;

    /**
     * \brief Validate and serialize Crypto API Management configuration.
     *
     * \return Two byte Crypto API configuration.
     */
    ByteVector serialize() const;
};

} // namespace crypto_api

// ----------------------------------------
// Authentication Counter and Limit (0x16)
// ----------------------------------------

namespace authentication_limit
{

/**
 * \brief Authentication Counter and Limit configuration.
 *
 * Configures the authentication counter options and the maximum permitted authentication count.
 */
struct LLA_CARDS_DESFIRE_API Configuration
{
    /**
     * \brief Enables AuthenticateEV2First.
     *
     * Bit 0 of AuthCtrOption.
     *
     * This option is disabled by default.
     */
    bool enableEV2First = false;

    /**
     * \brief Authentication counter limit.
     *
     * A value of zero disables the limit.
     *
     * Valid range : 0x00000000-0xFFFFFFFF.
     */
    std::uint32_t limit = 0;

    /**
     * \brief Validate and serialize Authentication Counter configuration.
     *
     * \return Six byte Authentication Counter configuration.
     */
    ByteVector serialize() const;
};

} // namespace authentication_limit

// ----------------------------------------
// HALT / Wake-Up (0x17)
// ----------------------------------------

namespace halt_wakeup
{

/**
 * \brief HALT and Wake-Up configuration.
 *
 * Requires exactly four bytes.
 *
 * This option is deliberately not interpreted by this library.
 * The caller is responsible for supplying a correctly formatted configuration according to the card specification.
 *
 * This option is only applicable at application level.
 */
struct LLA_CARDS_DESFIRE_API Configuration
{
    /**
     * \brief Raw HALT and Wake-Up configuration.
     *
     * Must contain exactly four bytes.
     */
    ByteVector rawData;

    /**
     * \brief Validate and serialize HALT and Wake-Up configuration.
     *
     * \return The supplied four byte configuration.
     */
    ByteVector serialize() const;
};

} // namespace halt_wakeup

// ----------------------------------------
// Lock Configuration (0xFF)
// ----------------------------------------

namespace locking
{

/**
 * \brief Lock Configuration.
 *
 * Configures the 24 bit configuration lock bitmap.
 *
 * Each bit corresponds to a SetConfiguration option.
 * 
 * A bit set to one locks the corresponding configuration.
 * A bit set to zero leaves it unchanged.
 */
struct LLA_CARDS_DESFIRE_API Configuration
{
    /**
     * \brief Three byte configuration lock bitmap.
     */
    std::array<std::uint8_t, 3> lockMap{};

    /**
     * \brief Serialize the Lock Configuration.
     *
     * \return Three byte lock bitmap.
     */
    ByteVector serialize() const;
};

} // namespace locking

// ----------------------------------------
// Serialized SetConfiguration payload
// ----------------------------------------

/**
 * \brief SetConfiguration command payload.
 *
 * Represents the option byte and associated configuration data.
 *
 * The selected level determines whether the option is applicable.
 * Options defined as PICC only must be considered RFU at application level.
 */
struct LLA_CARDS_DESFIRE_API Payload
{
    /**
     * \brief Configuration option.
     *
     * See configuration::option for supported option values.
     */
    std::uint8_t option = option::PICC_CONFIGURATION;

    /**
     * \brief Configuration data.
     *
     * Valid length : 1-33 bytes.
     */
    ByteVector data;

    /**
     * \brief Validate the complete payload.
     * 
     * Checks the data length, option specific constraints and compatibility with the currently selected level.
     *
     * \param selectedLevel Current PICC/application selection.
     * \param delegatedApplication Whether the selected application is a delegated application.
     *
     * \throw LibLogicalAccessException if invalid.
     */
    void validate(Level selectedLevel, bool delegatedApplication = false) const;
};

} // namespace configuration

} // namespace duox

} // namespace logicalaccess