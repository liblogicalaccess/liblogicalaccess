#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace logicalaccess
{

namespace duox_ev_charging
{

// ========================================
// ISO7816 and EV Charging protocol constants
// ========================================

/**
 * \brief ISO7816 and NXP EV Charging protocol constants.
 */
namespace protocol
{
// EV Charging proprietary commands
constexpr std::uint8_t CLA = 0x80U;

constexpr std::uint8_t INS_WRITE_DATA = 0x01U;
constexpr std::uint8_t INS_READ_DATA  = 0x02U;
constexpr std::uint8_t INS_ECDSA_SIGN = 0x03U;

// VDE_WriteData parameters
constexpr std::uint8_t WRITE_DATA_P1 = 0x06U;
constexpr std::uint8_t WRITE_DATA_P2 = 0x00U;
constexpr std::uint8_t LOCK_DATA_P2  = 0x01U;

// VDE_ReadData parameters
constexpr std::uint8_t READ_DATA_P2 = 0x00U;

// VDE_ECDSASign parameters
constexpr std::uint8_t ECDSA_SIGN_P1 = 0x0CU;
constexpr std::uint8_t ECDSA_SIGN_P2 = 0x09U;

// Expected encoded certificate sizes, depending on the card UID length
constexpr std::size_t UID7_CERTIFICATE_SIZE  = 193U;
constexpr std::size_t UID10_CERTIFICATE_SIZE = 199U;

// Additional data file capacity
constexpr std::size_t ADDITIONAL_DATA_FILE_SIZE = 1024U;

// Below are the identifiers of the pre-installed EV Charging application on MIFARE DUOX EV part types

// Pre-personalized EV Charging application identifiers
constexpr std::array<std::uint8_t, 16> APPLICATION_DF_NAME = {
    0xA0U, 0x00U, 0x00U, 0x08U, 0x45U, 0x00U, 0x00U, 0x00U,
    0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x01U};

// ISO file identifiers within the EV Charging application
constexpr std::uint16_t APPLICATION_ISO_FILE_ID     = 0x00DFU;
constexpr std::uint16_t CERTIFICATE_ISO_FILE_ID     = 0x00EFU;
constexpr std::uint16_t ADDITIONAL_DATA_ISO_FILE_ID = 0x01EFU;

} // namespace protocol

// ========================================
// APDU configuration
// ========================================

/**
 * \brief APDU length encoding format used by EV Charging commands.
 *
 * Short : One byte Lc and/or Le fields.
 * Extended : Three byte Lc and/or Le fields.
 */
enum class APDULengthFormat : std::uint8_t
{
    Short,
    Extended
};

// ========================================
// EV Charging application
// ========================================

/**
 * \brief File number identifying a file in the EV Charging application.
 *
 * The file number is provided in the P1 parameter of VDE_ReadData.
 */
enum class FileNo : std::uint8_t
{
    Certificate    = 0x00U,
    AdditionalData = 0x01U
};

/**
 * \brief Operation selector for the EV Charging VDE_WriteData command.
 */
enum class WriteDataOperation : std::uint8_t
{
    Write = protocol::WRITE_DATA_P2,
    Lock  = protocol::LOCK_DATA_P2
};

// ========================================
// Certificate
// ========================================

namespace certificate
{
// BER-TLV tags used in EV Charging certificates
constexpr std::uint16_t TAG_CERTIFICATE             = 0x7F21U;
constexpr std::uint16_t TAG_UID                     = 0x0093U;
constexpr std::uint16_t TAG_CA_ID                   = 0x0042U;
constexpr std::uint16_t TAG_SUBJECT_IDENTIFIER      = 0x5F20U;
constexpr std::uint16_t TAG_KEY_USAGE               = 0x0095U;
constexpr std::uint16_t TAG_EFFECTIVE_DATE          = 0x5F25U;
constexpr std::uint16_t TAG_EXPIRATION_DATE         = 0x5F24U;
constexpr std::uint16_t TAG_CA_SD_IMAGE_NUMBER      = 0x0045U;
constexpr std::uint16_t TAG_PUBLIC_KEY_DATA         = 0x7F49U;
constexpr std::uint16_t TAG_PUBLIC_KEY              = 0x00B0U;
constexpr std::uint16_t TAG_KEY_PARAMETER_REFERENCE = 0x00F0U;
constexpr std::uint16_t TAG_SIGNATURE               = 0x5F37U;

// Supported elliptic curve parameter reference
constexpr std::uint8_t KEY_PARAMETER_BRAINPOOL_P256R1 = 0x03U;

// Certificate field sizes
constexpr std::size_t CA_ID_SIZE = 7U;
constexpr std::size_t DATE_SIZE  = 4U;

} // namespace certificate

// ========================================
// Certificate cryptography
// ========================================

/**
 * \brief EV Charging cryptographic parameters
 */
namespace crypto
{
constexpr std::size_t HASH_SIZE        = 32U;
constexpr std::size_t COORDINATE_SIZE  = 32U;

// EV Charging cryptographic sizes
constexpr std::size_t CHALLENGE_SIZE  = HASH_SIZE;
constexpr std::size_t SIGNATURE_SIZE  = 2U * COORDINATE_SIZE;
constexpr std::size_t PUBLIC_KEY_SIZE = 1U + 2U * COORDINATE_SIZE;

} // namespace crypto

/**
 * \brief Supported key parameter references in EV Charging certificates.
 *
 * Identifies the elliptic curve domain parameters associated with the certificate public key.
 */
enum class EVChargingKeyParameterReference : std::uint8_t
{
    BrainpoolP256r1 = certificate::KEY_PARAMETER_BRAINPOOL_P256R1
};

} // namespace duox_ev_charging

} // namespace logicalaccess