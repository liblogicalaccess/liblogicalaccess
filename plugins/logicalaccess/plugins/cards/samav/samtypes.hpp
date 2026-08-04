#ifndef LOGICALACCESS_SAMTYPES_HPP
#define LOGICALACCESS_SAMTYPES_HPP

#include <cstddef>
#include <cstdint>
#include <string>

namespace logicalaccess
{
namespace sam
{

constexpr std::size_t APDU_COMMAND_HEADER_SIZE  = 0x04; // CLA INS P1 P2
constexpr std::size_t APDU_HEADER_SIZE          = 0x05; // CLA INS P1 P2 Lc
constexpr std::size_t APDU_HEADER_WITH_LE_SIZE  = APDU_HEADER_SIZE + 1u; // CLA INS P1 P2 Lc Le
constexpr std::size_t APDU_LC_INDEX             = 0x04;
constexpr std::size_t MAX_APDU_SIZE             = 0xFF;
constexpr std::size_t MAX_APDU_PAYLOAD_SIZE     = MAX_APDU_SIZE - APDU_HEADER_SIZE;
constexpr std::size_t MAX_SECURE_APDU_DATA_SIZE = 0xF0;

constexpr unsigned char AES_BLOCK_SIZE   = 16;
constexpr unsigned char AES_128_KEY_SIZE = 16;
constexpr unsigned char AES_192_KEY_SIZE = 24;
constexpr unsigned char AES_256_KEY_SIZE = 32;
constexpr unsigned char STATUS_WORD_SIZE = 2;
constexpr unsigned char MAC_SIZE         = 8;

// APDU format abstraction
enum class ApduFormat : unsigned char
{
    SingleFrame,   // Single short APDU (command fits in one frame and payload is < 241 bytes when protected)
    Chained,       // Command is transmitted over one or more extended APDUs (payload exceeds the Standard 240 bytes limit)
    ChainedWithLe  // Same as Extended, with an Le field in the final APDU
};

// APDU result wrapper
struct ProtectedApdu
{
    ByteVector encData;
    ByteVector mac;
    bool hasLe = false;
};

struct ApduInfo
{
    bool hasLc = false;
    bool hasLe = false;
};

// Host communication mode
enum class HostMode : unsigned char
{
    None        = 0xFF, // No host mode is active (host authentication not yet established)
    Plain       = 0x00,
    MAC         = 0x01,
    FullProtect = 0x02
};

// Hash algorithms (SAM AV2 / PKI)
enum class HashAlgo : unsigned char
{
    SHA1   = 0x00,
    SHA224 = 0x01,
    RFU    = 0x02,
    SHA256 = 0x03
};

constexpr bool isValidAESKeySize(std::size_t size) noexcept
{
    return size == AES_128_KEY_SIZE || size == AES_192_KEY_SIZE || size == AES_256_KEY_SIZE;
}

constexpr bool isSupportedHashAlgo(unsigned char algo) noexcept
{
    return algo == static_cast<unsigned char>(HashAlgo::SHA1) ||
           algo == static_cast<unsigned char>(HashAlgo::SHA224) ||
           algo == static_cast<unsigned char>(HashAlgo::SHA256);
}

constexpr unsigned char expectedHashSize(unsigned char algo) noexcept
{
    switch (algo)
    {
    case static_cast<unsigned char>(HashAlgo::SHA1): return 20;
    case static_cast<unsigned char>(HashAlgo::SHA224): return 28;
    case static_cast<unsigned char>(HashAlgo::SHA256): return 32;
    default: return 0;
    }
}

// APDU chaining layout
struct ChainingLayout
{
    static constexpr unsigned char NoIndex = 0xFF;

    unsigned char modeIndex;
    unsigned char lastFrameIndex;

    constexpr bool hasModeIndex() const noexcept
    {
        return modeIndex != NoIndex;
    }

    constexpr bool hasLastFrameIndex() const noexcept
    {
        return lastFrameIndex != NoIndex;
    }

    constexpr bool hasChaining() const noexcept
    {
        return hasModeIndex() && hasLastFrameIndex();
    }
};

// Predefined layouts
static constexpr ChainingLayout PKI_ECC_LAYOUT = {2, 3}; // PKI and ECC commands
static constexpr ChainingLayout EMV_LAYOUT     = {3, 2}; // EMV commands

// Fallback layout for commands without chaining fields
// Keeps P1/P2 unchanged when no chaining information is encoded
static constexpr ChainingLayout NO_LAYOUT      = {ChainingLayout::NoIndex, ChainingLayout::NoIndex};

// Optional AEK/VAEK (replaces pointers)
struct AEKVAEK
{
    unsigned char keyNoAEK = 0;
    unsigned char keyVAEK  = 0;
    bool valid       = false;

    constexpr AEKVAEK() = default;

    constexpr AEKVAEK(unsigned char aek, unsigned char vaek) : keyNoAEK(aek), keyVAEK(vaek), valid(true)
    {

    }

    constexpr explicit operator bool() const noexcept
    {
        return valid;
    }
};

// Utility helpers
inline void appendUInt32BE(ByteVector &out, std::uint32_t value) noexcept
{
    out.push_back(static_cast<unsigned char>((value >> 24) & 0xFF));
    out.push_back(static_cast<unsigned char>((value >> 16) & 0xFF));
    out.push_back(static_cast<unsigned char>((value >> 8) & 0xFF));
    out.push_back(static_cast<unsigned char>(value & 0xFF));
}

inline void appendUInt16BE(ByteVector &out, std::uint16_t value) noexcept
{
    out.push_back(static_cast<unsigned char>((value >> 8) & 0xFF));
    out.push_back(static_cast<unsigned char>(value & 0xFF));
}

// TODO Replace with ISO7816Response once SAM commands migrate to that abstraction
[[nodiscard]]
inline bool tryParseStatusWord(const ByteVector &response, std::uint16_t &statusWord) noexcept
{
    if (response.size() < STATUS_WORD_SIZE)
        return false;

    statusWord = (static_cast<std::uint16_t>(response[response.size() - STATUS_WORD_SIZE]) << 8) | response.back();

    return true;
}

inline std::uint16_t parseStatusWord(const ByteVector &response) noexcept
{
    std::uint16_t sw = 0;
    (void)tryParseStatusWord(response, sw);
    return sw;
}

// Constants
constexpr unsigned char toByte(HostMode mode) noexcept
{
    return static_cast<unsigned char>(mode);
}

inline std::string errorMessage(const char *function, const std::string &message)
{
    return std::string(function) + " : " + message;
}

namespace pki
{
constexpr unsigned short ConfigDisableBit = 0x0004;
}

namespace sw
{
constexpr unsigned char SuccessSW1 = 0x90;
constexpr unsigned char SuccessSW2  = 0x00;
constexpr unsigned char MoreDataSW2 = 0xAF;
}

constexpr bool isSuccess(unsigned char sw1, unsigned char sw2) noexcept
{
    return sw1 == sw::SuccessSW1 && sw2 == sw::SuccessSW2;
}

constexpr bool hasMoreData(unsigned char sw1, unsigned char sw2) noexcept
{
    return sw1 == sw::SuccessSW1 && sw2 == sw::MoreDataSW2;
}

constexpr bool validState(unsigned char sw1, unsigned char sw2) noexcept
{
    return sw1 == sw::SuccessSW1 && (sw2 == sw::SuccessSW2 || sw2 == sw::MoreDataSW2);
}

namespace chaining
{
constexpr unsigned char Continue = sw::MoreDataSW2;
constexpr unsigned char End      = sw::SuccessSW2;
}

namespace iso7816
{
constexpr unsigned char LeResponse = 0x00;
}

#ifndef SWIG
namespace ins
{
namespace host
{
constexpr unsigned char AuthenticateHost = 0xA4;
}

namespace key
{
constexpr unsigned char GetKeyEntry            = 0x64;
constexpr unsigned char ChangeKeyEntry         = 0xC1;
constexpr unsigned char DisableKeyEntryOffline = 0xD8;
constexpr unsigned char EncipherKeyEntry       = 0xE1;
constexpr unsigned char DumpSecretKey          = 0xD6;
}

namespace kuc
{
constexpr unsigned char GetEntry    = 0x6C;
constexpr unsigned char ChangeEntry = 0xCC;
}

namespace offline
{
constexpr unsigned char ActivateKey  = 0x01;
constexpr unsigned char DecipherData = 0x0D;
constexpr unsigned char EncipherData = 0x0E;
}

namespace pki
{
constexpr unsigned char GenerateKeyPair    = 0x15;
constexpr unsigned char ImportKey          = 0x19;
constexpr unsigned char ExportPrivateKey   = 0x1F;
constexpr unsigned char ExportPublicKey    = 0x18;
constexpr unsigned char UpdateKeyEntries   = 0x1D;
constexpr unsigned char EncipherKeyEntries = 0x12;
constexpr unsigned char GenerateHash       = 0x17;
constexpr unsigned char GenerateSignature  = 0x16;
constexpr unsigned char SendSignature      = 0x1A;
constexpr unsigned char VerifySignature    = 0x1B;
constexpr unsigned char EncipherData       = 0x13;
constexpr unsigned char DecipherData       = 0x14;
constexpr unsigned char ImportECCKey       = 0x21;
constexpr unsigned char ImportECCCurve     = 0x22;
constexpr unsigned char ExportECCPublicKey = 0x23;
constexpr unsigned char VerifyECCSignature = 0x20;
}

namespace emv
{
constexpr unsigned char ImportCaPk         = 0x24;
constexpr unsigned char RemoveCaPk         = 0x2F;
constexpr unsigned char ExportCaPk         = 0x3D;
constexpr unsigned char LoadIssuerPk       = 0x27;
constexpr unsigned char LoadIccPk          = 0x28;
constexpr unsigned char RecoverStaticData  = 0x29;
constexpr unsigned char RecoverDynamicData = 0x2A;
constexpr unsigned char EncipherPin        = 0x2B;
}
}
#endif

} // namespace sam
} // namespace logicalaccess

#endif /* LOGICALACCESS_SAMTYPES_HPP */