#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include <logicalaccess/lla_fwd.hpp>
#include <logicalaccess/plugins/cards/desfire/lla_cards_desfire_api.hpp>

namespace logicalaccess
{

namespace desfire
{

namespace configuration
{

// ----------------------------------------
// Virtual Card Type Identifier
// ----------------------------------------

/**
 * \brief Virtual Card Type Identifier (VCTID).
 *
 * The VCTID occupies the six least significant bits of the VC information byte exchanged during virtual card selection.
 *
 * Values not explicitly named here may still be represented using a static_cast from their underlying byte value,
 * provided they are valid according to the applicable protocol specification.
 */
enum class VCTypeIdentifier : std::uint8_t
{
    RESERVED                      = 0x00,
    MIFARE_PLUS_SL1               = 0x01,
    MIFARE_PLUS_SL2               = 0x02,
    MIFARE_PLUS_SL3               = 0x03,
    MIFARE_DESFIRE                = 0x04,
    MIFARE_ULTRALIGHT             = 0x05,
    MIFARE_ULTRALIGHT_C           = 0x06,
    MIFARE_PLUS_SL0               = 0x07,
    DUOX                          = 0x09,
    MIFARE_PLUS_SL1_SWITCH_HIGHER = 0x21,
    UNSUPPORTED_IID               = 0x3F
};

/**
 * \brief Virtual Card UID length encoding.
 *
 * These values occupy bits 7-6 of the VC information byte
 */
enum class VCUIDLength : std::uint8_t
{
    UID_7_BYTES  = 0x00,
    UID_10_BYTES = 0x40,
    UID_4_BYTES  = 0x80,
    RFU          = 0xC0
};

// ----------------------------------------
// PD capability information
// Info | PDCap1.1 | PDCap1.2
// ----------------------------------------

/**
 * \brief Information exchanged during virtual card selection
 */
struct LLA_CARDS_DESFIRE_API PDCapabilityInformation
{
    VCUIDLength uidLength = VCUIDLength::UID_7_BYTES;
    VCTypeIdentifier vcType = VCTypeIdentifier::MIFARE_DESFIRE;

    /**
     * \brief Chipmaker defined capability byte.
     * 
     * Interpretation depends on the VCTID.
     */
    std::uint8_t pdCap1_1 = 0x00;

    /**
     * \brief Service operator defined capability byte.
     *
     * Its structure is described by pd_cap1_2::Configuration.
     */
    std::uint8_t pdCap1_2 = 0x00;

    /**
     * \brief Serialize the capability information.
     *
     * \return Three byte capability information.
     */
    ByteVector serialize() const;
};

// ----------------------------------------
// PDCap1.2 service operator defined capability fields
// ----------------------------------------

namespace pd_cap1_2
{

/**
 * \brief Card memory size encoded in PDCap1.2 bits 2-0
 */
enum class CardSize : std::uint8_t
{
    LESS_THAN_1KB = 0x00,
    KB_1          = 0x01,
    KB_2          = 0x02,
    KB_4          = 0x03,
    KB_8          = 0x04,
    KB_16         = 0x05,
    KB_32         = 0x06,
    RESERVED      = 0x07
};

/**
 * \brief Proximity Check support encoded in PDCap1.2 bits 4-3.
 */
enum class ProximityCheck : std::uint8_t
{
    NOT_SUPPORTED = 0x00,
    SUPPORTED     = 0x08,
    MANDATORY     = 0x10,
    RESERVED      = 0x18
};

struct Configuration
{
    CardSize cardSize             = CardSize::LESS_THAN_1KB;
    ProximityCheck proximityCheck = ProximityCheck::NOT_SUPPORTED;

    /**
     * \brief Commander Keyset ID.
     *
     * Valid range : 0x00-0x07.
     * Encoded in bits 7-5.
     */
    std::uint8_t commanderKeysetId = 0x00;

    /**
     * \brief Serialize the PDCap1.2 byte.
     *
     * \return One byte PDCap1.2 representation.
     */
    std::uint8_t serialize() const;
};

} // namespace pd_cap1_2

// ----------------------------------------
// Transaction Timer : PDCap2.2
// ----------------------------------------

/**
 * \brief Transaction Timer values encoded in PDCap2.2.
 *
 * The timer is disabled when its value is zero.
 *
 * Once enabled, the card automatically resets when the configured transaction duration threshold is exceeded
 */
enum class TransactionTimer : std::uint8_t
{
    DISABLED                         = 0x00,
    ONE_TO_THREE_SECONDS             = 0x01,
    TEN_TO_THIRTY_SECONDS            = 0x02,
    HUNDRED_TO_THREE_HUNDRED_SECONDS = 0x03
};

// ----------------------------------------
// PD authentication capabilities :
// PDCap2.1 | PDCap2.2 | PDCap2.3 | PDCap2.4 | PDCap2.5 | PDCap2.6
// ----------------------------------------

/**
 * \brief PD capabilities exchanged during symmetric authentication.
 */
struct LLA_CARDS_DESFIRE_API PDAuthenticationCapabilities
{
    /**
     * \brief Chipmaker defined capability.
     *
     * Reserved for future use.
     */
    std::uint8_t pdCap2_1 = 0x00;

    /**
     * \brief Transaction Timer configuration.
     */
    TransactionTimer pdCap2_2 = TransactionTimer::DISABLED;

    /**
     * \brief Chipmaker defined capability.
     *
     * Reserved for future use.
     */
    std::uint8_t pdCap2_3 = 0x00;

    /**
     * \brief Chipmaker defined capability.
     *
     * Reserved for future use.
     */
    std::uint8_t pdCap2_4 = 0x00;

    /**
     * \brief Service operator defined capability.
     */
    std::uint8_t pdCap2_5 = 0x00;

    /**
     * \brief Service operator defined capability.
     */
    std::uint8_t pdCap2_6 = 0x00;

    /**
     * \brief Serialize all six PD authentication capability bytes.
     *
     * \return Six byte capability representation.
     */
    ByteVector serialize() const;
};

// ----------------------------------------
// PCD authentication capabilities :
// PCDCap2.1 ... PCDCap2.6
// ----------------------------------------

/**
 * \brief PCD capabilities supplied during symmetric authentication.
 *
 * The six capability bytes are currently reserved and shall be zero.
 *
 * PCD may omit trailing capability bytes. If bytes are omitted, all subsequent bytes are implicitly interpreted as zero.
 */
struct LLA_CARDS_DESFIRE_API PCDAuthenticationCapabilities
{
    static constexpr std::size_t CAPABILITY_COUNT = 6;

    /**
     * \brief PCD capability bytes.
     *
     * All six bytes shall currently be zero.
     */
    std::array<std::uint8_t, CAPABILITY_COUNT> bytes{};

    /**
     * \brief Validate the PCD capability bytes.
     */
    void validate() const;

    /**
     * \brief Serialize the PCD capability bytes.
     *
     * \param transmittedLength Number of bytes to transmit.
     *
     * Valid range : 0-6.
     *
     * Omitted trailing bytes are interpreted as zero by the PD.
     *
     * \return Serialized capability bytes.
     */
    ByteVector serialize(std::size_t transmittedLength = CAPABILITY_COUNT) const;
};

// ----------------------------------------
// Protocol Options
// ----------------------------------------

namespace protocol_options
{

/**
 * \brief ProtocolOptionsA bit definitions.
 *
 * These bits describe the cryptographic protocols supported over NFC or I2C.
 * 
 * Their presence in this definition does not imply that every card supports all corresponding protocols.
 */
constexpr std::uint8_t AES_AUTHENTICATION = 0x01;
constexpr std::uint8_t ECC_MUTUAL_AUTHENTICATION = 0x02;
constexpr std::uint8_t ECC_CARD_UNILATERAL_AUTHENTICATION = 0x04;
constexpr std::uint8_t ECDSA_SIGN = 0x08;

/**
 * \brief Mask of all currently defined ProtocolOptionsA bits.
 * 
 * Used to reject reserved bits during serialization.
 * This mask does not represent the capabilities of a particular card.
 */
constexpr std::uint8_t VALID_MASK_A = 0x0F;

/**
 * \brief Mask of currently defined ProtocolOptionsB bits.
 *
 * All bits in ProtocolOptionsB are RFU.
 */
constexpr std::uint8_t VALID_MASK_B = 0x00;

/**
 * \brief ProtocolOptions configuration.
 *
 * Used by NFC Management and I2C Management.
 *
 * The default configuration enables all currently defined ProtocolOptionsA capabilities.
 *
 * ProtocolOptionsB defaults to zero because all its bits are currently RFU.
 */
struct LLA_CARDS_DESFIRE_API Configuration
{
    /**
     * \brief ProtocolOptions byte A.
     *
     * Defaults to all currently defined protocol options enabled.
     */
    std::uint8_t protocolOptionsA = VALID_MASK_A;

    /**
     * \brief ProtocolOptions byte B.
     *
     * All bits are currently RFU and must remain zero.
     */
    std::uint8_t protocolOptionsB = 0x00;

    /**
     * \brief Validate and serialize ProtocolOptions.
     *
     * \return Two byte ProtocolOptions representation.
     */
    ByteVector serialize() const;
};

} // namespace protocol_options

} // namespace configuration

} // namespace desfire

} // namespace logicalaccess