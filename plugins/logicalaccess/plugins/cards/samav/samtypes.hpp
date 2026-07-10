#ifndef LOGICALACCESS_SAMTYPES_HPP
#define LOGICALACCESS_SAMTYPES_HPP

#include <cstdint>

namespace logicalaccess
{
namespace sam
{

constexpr std::size_t MAX_APDU_DATA_SIZE = 0xFF;
constexpr std::size_t SAM_SECURE_CHANNEL_MAX_PLAIN_LC = 0xF0;

constexpr unsigned char AES_BLOCK_SIZE = 16;
constexpr unsigned char STATUS_WORD_SIZE = 2;
constexpr unsigned char MAC_SIZE = 8;

// APDU format abstraction
enum class ApduFormat : unsigned char
{
    Standard,            // short APDU (< 241 bytes)
    Extended,            // extended APDU
    ExtendedWithLe,      // extended APDU with Le
    ExtendedResponseOnly // extended APDU for responses only
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

constexpr bool isSupportedHashAlgo(unsigned char algo)
{
    return algo == static_cast<unsigned char>(HashAlgo::SHA1) ||
           algo == static_cast<unsigned char>(HashAlgo::SHA224) ||
           algo == static_cast<unsigned char>(HashAlgo::SHA256);
}

constexpr unsigned char expectedHashSize(unsigned char algo)
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
    unsigned char modeIndex;
    unsigned char lastFrameIndex;
};

// Predefined layouts
static constexpr ChainingLayout PKI_ECC_LAYOUT = {2, 3}; //PKI and ECC commands
static constexpr ChainingLayout EMV_LAYOUT     = {3, 2}; //EMV commands

// Optional AEK/VAEK (replaces pointers)
struct AEKVAEK
{
    uint8_t keyNoAEK = 0;
    uint8_t keyVAEK  = 0;
    bool valid       = false;

    constexpr AEKVAEK() = default;

    constexpr AEKVAEK(uint8_t aek, uint8_t vaek) : keyNoAEK(aek), keyVAEK(vaek), valid(true)
    {

    }

    constexpr explicit operator bool() const noexcept
    {
        return valid;
    }
};

// Utility helpers
inline void appendUInt32BE(ByteVector &out, uint32_t value)
{
    out.push_back(static_cast<unsigned char>((value >> 24) & 0xFF));
    out.push_back(static_cast<unsigned char>((value >> 16) & 0xFF));
    out.push_back(static_cast<unsigned char>((value >> 8) & 0xFF));
    out.push_back(static_cast<unsigned char>(value & 0xFF));
}

inline void appendUInt16BE(ByteVector &out, uint16_t value)
{
    out.push_back(static_cast<unsigned char>((value >> 8) & 0xFF));
    out.push_back(static_cast<unsigned char>(value & 0xFF));
}

inline uint16_t parseStatusWord(const ByteVector &response)
{
    return (static_cast<uint16_t>(response[response.size() - 2]) << 8) |
           static_cast<uint16_t>(response[response.size() - 1]);
}

// Constants
constexpr unsigned char toByte(HostMode mode)
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
constexpr uint16_t Success = 0x9000;
constexpr unsigned char SuccessSW1 = 0x90;
constexpr unsigned char SuccessSW2  = 0x00;
constexpr unsigned char MoreDataSW2 = 0xAF;
}

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

} // namespace sam
} // namespace logicalaccess

#endif /* LOGICALACCESS_SAMTYPES_HPP */