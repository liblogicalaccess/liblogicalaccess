#ifndef LOGICALACCESS_SAMTYPES_HPP
#define LOGICALACCESS_SAMTYPES_HPP

#include <cstdint>

namespace logicalaccess
{
namespace sam
{

constexpr std::size_t MAX_APDU_DATA_SIZE = 0xFF;
constexpr std::size_t SAM_SECURE_CHANNEL_MAX_PLAIN_LC = 0xF0;

// APDU format abstraction
enum class ApduFormat : unsigned char
{
    Standard,      // short APDU (< 241 bytes)
    Extended,      // extended APDU
    ExtendedWithLe // extended APDU with Le
};

// APDU result wrapper
struct ApduResult
{
    ByteVector encData;
    ByteVector mac;
    bool hasLe = false;
};

// Host communication mode
enum class HostMode : unsigned char
{
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
static constexpr ChainingLayout PKI_ECC_LAYOUT = {2, 3};
static constexpr ChainingLayout EMV_LAYOUT     = {3, 2};

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
    if (response.size() < 2)
        return 0;

    return (static_cast<uint16_t>(response[response.size() - 2]) << 8) |
           static_cast<uint16_t>(response[response.size() - 1]);
}

// Constants
constexpr unsigned char toByte(HostMode mode)
{
    return static_cast<unsigned char>(mode);
}

} // namespace sam
} // namespace logicalaccess

#endif /* LOGICALACCESS_SAMTYPES_HPP */