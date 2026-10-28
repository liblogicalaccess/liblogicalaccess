#pragma once

#include <cstddef>
#include <cstdint>

namespace logicalaccess
{
namespace duox
{

// ---------------------------------------
// DUOX EV2 authentication
// ---------------------------------------

namespace authentication
{
constexpr std::uint8_t AUTHENTICATE_EV2_FIRST = 0x71;

constexpr std::uint8_t KEYNO_SECONDARY_APPLICATION = 0x80;
constexpr std::uint8_t KEYNO_RESERVED              = 0x40;
constexpr std::uint8_t KEYNO_MASK                  = 0x3F;

constexpr std::size_t AES_BLOCK_SIZE = 16;

constexpr std::size_t EV2_RANDOM_SIZE  = 16;
constexpr std::size_t EV2_TI_SIZE      = 4;
constexpr std::size_t EV2_PDCAP2_SIZE  = 6;
constexpr std::size_t EV2_PCDCAP2_SIZE = 6;

constexpr std::size_t EV2_PART1_RESPONSE_SIZE = 16;
constexpr std::size_t EV2_PART2_COMMAND_SIZE  = 32;
constexpr std::size_t EV2_PART2_RESPONSE_SIZE = 32;
} // namespace authentication

// ---------------------------------------
// Transaction MAC
// ---------------------------------------

namespace tmac
{
constexpr unsigned char FILE_OPTION_TMI_EXCLUSION = 0x40;

constexpr unsigned char KEY_OPTION_NONE     = 0x00;
constexpr unsigned char KEY_OPTION_AES128   = 0x02;
constexpr unsigned char KEY_OPTION_AES256   = 0x03;
constexpr unsigned char KEY_OPTION_SIGNATURE = 0x80;
} // namespace tmac

// ---------------------------------------
// DUOX instruction codes (INS)
// ---------------------------------------

namespace ins
{
constexpr std::uint8_t GET_KEY_SETTINGS   = 0x45;
constexpr std::uint8_t MANAGE_KEY_PAIR    = 0x46;
constexpr std::uint8_t EXPORT_KEY         = 0x47;
constexpr std::uint8_t MANAGE_CA_ROOT_KEY = 0x48;
constexpr std::uint8_t CHANGE_KEY         = 0xC4;
constexpr std::uint8_t CHANGE_KEY_EV2     = 0xC6;

constexpr std::uint8_t AUTHENTICATE_EV2_NON_FIRST = 0x77;
} // namespace ins

// ---------------------------------------
// Key and application limits
// ---------------------------------------

namespace limits
{
constexpr std::uint8_t PICC_LEVEL_AID  = 0x00;
constexpr std::uint8_t MAX_PICC_KEY_NO = 0x01;
constexpr std::uint8_t MAX_KEY_NO      = 0x04;
} // namespace limits

// ---------------------------------------
// ExportKey options
// ---------------------------------------

namespace export_key
{
constexpr std::uint8_t OPTION_CA_ROOT_KEY = 0x01;
} // namespace export_key

// ---------------------------------------
// ECC key encoding
// ---------------------------------------

namespace ecc
{
constexpr std::size_t PRIVATE_KEY_SIZE = 32;

/**
 * P-256 uncompressed public key :
 *   0x04 || X || Y
 */
constexpr std::size_t PUBLIC_KEY_SIZE            = 65;
constexpr std::uint8_t UNCOMPRESSED_POINT_PREFIX = 0x04;
} // namespace ecc

// ---------------------------------------
// Access rights and key policy
// ---------------------------------------

namespace access
{
/**
 * WriteAccess :
 *   bits 0-3 : WriteAR
 *   bits 4-5 : communication mode
 *   bits 6-7 : RFU
 */
constexpr std::uint8_t COM_MODE_MASK = 0x30;
constexpr std::uint8_t RFU_MASK      = 0xC0;

/** AccessRights bits 14 and 15 are RFU */
constexpr std::uint16_t RIGHTS_VALID_MASK = 0x3FFF;

/*
 * CRLFile :
 *   bit 7    : CRL enabled
 *   bits 5-6 : RFU
 *   bits 0-4 : CRL File number when enabled, otherwise zero
 */
constexpr std::uint8_t CRL_FILE_ENABLED_MASK = 0x80;
constexpr std::uint8_t CRL_FILE_NUMBER_MASK  = 0x1F;
constexpr std::uint8_t CRL_FILE_RFU_MASK     = 0x60;

constexpr std::uint32_t AID_3_BYTE_MASK = 0x00FFFFFF;

constexpr std::uint16_t KEY_POLICY_VALID_MASK =
    static_cast<std::uint16_t>((1u << 4) | // CryptoRequest
                               (1u << 5) | // SecureDynamicMessaging
                               (1u << 6) | // TransactionSignature
                               (1u << 7) | // MutualAuthentication
                               (1u << 8) | // UnilateralAuthentication
                               (1u << 15)  // FreezeKeyUsageCounter
    );
} // namespace access

// ---------------------------------------
// ISO7816 / DUOX status words
// ---------------------------------------

struct ExpectedStatus
{
    std::uint8_t sw1;
    std::uint8_t sw2;
};

namespace status
{
constexpr ExpectedStatus SUCCESS               = {0x90, 0x00};
constexpr ExpectedStatus DUOX_SUCCESS          = {0x91, 0x00};
constexpr ExpectedStatus DUOX_ADDITIONAL_FRAME = {0x91, DF_INS_ADDITIONAL_FRAME};
constexpr ExpectedStatus DUOX_FILE_LIMITED_FUNCTIONALITY = {0x62, 0x83};
} // namespace status

} // namespace duox
} // namespace logicalaccess