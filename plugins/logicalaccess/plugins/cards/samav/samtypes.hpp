#ifndef LOGICALACCESS_SAMTYPES_HPP
#define LOGICALACCESS_SAMTYPES_HPP

#include <cstddef>
#include <cstdint>
#include <string>
#include <memory>

namespace logicalaccess
{

class SAMBasicKeyEntry;

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

// Key entry update descriptor used by PKI_UpdateKeyEntries
struct SAMKeyEntryUpdate
{
    unsigned char keyNo;
    std::shared_ptr<SAMBasicKeyEntry> entry;
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

/**
 * \brief PKI key entry configuration settings.
 *
 * This is the 16 bit PKI_SET configuration.
 * 
 * Bits 10 to 15 are RFU and shall be set to zero.
 */
struct PKISet
{
    using value_type = std::uint16_t;

    static constexpr value_type PrivateKey         = 0x0001;
    static constexpr value_type AllowPrivateExport = 0x0002;
    static constexpr value_type Disable            = 0x0004;
    static constexpr value_type DisableEncryption  = 0x0008;
    static constexpr value_type DisableSignature   = 0x0010;
    static constexpr value_type UpdateKeyEntries   = 0x0020;
    static constexpr value_type CRT                = 0x0040;
    static constexpr value_type EncipherKeyEntries = 0x0080;
    static constexpr value_type ForceHostUsage     = 0x0100;
    static constexpr value_type ForceHostChange    = 0x0200;

    static constexpr value_type RFUMask   = 0xFC00;

    value_type value = 0;

    constexpr PKISet() noexcept = default;

    explicit constexpr PKISet(value_type value) noexcept
        : value(value)
    {
    }

    [[nodiscard]]
    static constexpr bool isValidRaw(value_type value) noexcept
    {
        return (value & RFUMask) == 0;
    }

    [[nodiscard]]
    constexpr bool isValid() const noexcept
    {
        return isValidRaw(value);
    }

    [[nodiscard]]
    constexpr value_type raw() const noexcept
    {
        return value;
    }

    constexpr void reset() noexcept
    {
        value = 0;
    }

    constexpr void setRaw(value_type newValue) noexcept
    {
        value = newValue;
    }

    constexpr void setPrivateKey(bool enabled) noexcept
    {
        setFlag(PrivateKey, enabled);
    }

    constexpr void setAllowPrivateExport(bool enabled) noexcept
    {
        setFlag(AllowPrivateExport, enabled);
    }

    constexpr void setDisabled(bool enabled) noexcept
    {
        setFlag(Disable, enabled);
    }

    constexpr void setEncryptionDisabled(bool enabled) noexcept
    {
        setFlag(DisableEncryption, enabled);
    }

    constexpr void setSignatureDisabled(bool enabled) noexcept
    {
        setFlag(DisableSignature, enabled);
    }

    constexpr void setUpdateKeyEntries(bool enabled) noexcept
    {
        setFlag(UpdateKeyEntries, enabled);
    }

    constexpr void setCRT(bool enabled) noexcept
    {
        setFlag(CRT, enabled);
    }

    constexpr void setEncipherKeyEntries(bool enabled) noexcept
    {
        setFlag(EncipherKeyEntries, enabled);
    }

    constexpr void setForceHostUsage(bool enabled) noexcept
    {
        setFlag(ForceHostUsage, enabled);
    }

    constexpr void setForceHostChange(bool enabled) noexcept
    {
        setFlag(ForceHostChange, enabled);
    }

    [[nodiscard]]
    constexpr bool privateKeyIncluded() const noexcept
    {
        return hasFlag(PrivateKey);
    }

    [[nodiscard]]
    constexpr bool privateKeyExportAllowed() const noexcept
    {
        return hasFlag(AllowPrivateExport);
    }

    [[nodiscard]]
    constexpr bool disabled() const noexcept
    {
        return hasFlag(Disable);
    }

    [[nodiscard]]
    constexpr bool encryptionDisabled() const noexcept
    {
        return hasFlag(DisableEncryption);
    }

    [[nodiscard]]
    constexpr bool signatureDisabled() const noexcept
    {
        return hasFlag(DisableSignature);
    }

    [[nodiscard]]
    constexpr bool updateKeyEntriesEnabled() const noexcept
    {
        return hasFlag(UpdateKeyEntries);
    }

    [[nodiscard]]
    constexpr bool crtRepresentation() const noexcept
    {
        return hasFlag(CRT);
    }

    [[nodiscard]]
    constexpr bool encipherKeyEntriesEnabled() const noexcept
    {
        return hasFlag(EncipherKeyEntries);
    }

    [[nodiscard]]
    constexpr bool hostUsageForced() const noexcept
    {
        return hasFlag(ForceHostUsage);
    }

    [[nodiscard]]
    constexpr bool hostChangeForced() const noexcept
    {
        return hasFlag(ForceHostChange);
    }

    [[nodiscard]]
    constexpr bool isValidForKeyEntry(unsigned char keyNo) const noexcept
    {
        // SAM AV3 PKI key entries are 0x00 to 0x02
        if (keyNo > 0x02)
            return false;

        // Bits 10 to 15 are RFU and must be set 0
        if (!isValid())
            return false;

        // Key entry 0x02 is public key only
        if (keyNo == 0x02 && privateKeyIncluded())
            return false;

        return true;
    }

    /**
     * \brief Serialize the PKI_SET value in SAM command byte order.
     *
     * PKI_SET is encoded as a 16 bit big-endian value in the corresponding SAM command payload.
     * 
     * This function doesn't validate the configuration !
     * Callers requiring a valid PKI_SET must check isValid() or isValidForKeyEntry() before serialization.
     */
    void appendTo(ByteVector &output) const
    {
        sam::appendUInt16BE(output, value);
    }

  private:
    [[nodiscard]]
    constexpr bool hasFlag(value_type flag) const noexcept
    {
        return (value & flag) != 0;
    }

    constexpr void setFlag(value_type flag, bool enabled) noexcept
    {
        if (enabled)
            value |= flag;
        else
            value &= static_cast<value_type>(~flag);
    }
};

/**
 * \brief RSA key data returned by the SAM AV3 PKI_ExportPrivateKey command.
 *
 * The response contains the PKI key entry configuration and RSA key components.
 * 
 * keyNoAEK/keyVerAEK fields are present only when the command was executed with P2 bit 7 set.
 */
struct ExportedPrivateKey
{
    PKISet config;

    unsigned char keyNoCEK  = 0;
    unsigned char keyVerCEK = 0;
    unsigned char refNoKUC  = 0;

    bool hasAccessKey       = false;
    unsigned char keyNoAEK  = 0;
    unsigned char keyVerAEK = 0;

    std::uint16_t nLen = 0;
    std::uint16_t eLen = 0;
    std::uint16_t pLen = 0;
    std::uint16_t qLen = 0;

    ByteVector n;
    ByteVector e;
    ByteVector p;
    ByteVector q;
    ByteVector dP;
    ByteVector dQ;
    ByteVector ipq;
};

/**
 * \brief Public RSA key exported from a SAM AV3 PKI key entry.
 *
 * Represents the decoded response of PKI_ExportPublicKey.
 */
struct ExportedPublicKey
{
    PKISet config{};

    unsigned char keyNoCEK = 0;
    unsigned char keyVCEK  = 0;
    unsigned char refNoKUC = 0;

    bool hasAccessKey      = false;
    unsigned char keyNoAEK = 0;
    unsigned char keyVAEK  = 0;

    std::uint16_t nLen = 0;
    std::uint16_t eLen = 0;

    ByteVector n;
    ByteVector e;

    [[nodiscard]]
    bool hasValidLengths() const noexcept
    {
        return n.size() == nLen && e.size() == eLen;
    }

    [[nodiscard]]
    bool hasPublicKey() const noexcept
    {
        return !n.empty() && !e.empty();
    }
};

} // namespace pki

namespace sw
{
constexpr unsigned char SuccessSW1 = 0x90;
constexpr unsigned char SuccessSW2  = 0x00;
constexpr unsigned char MoreDataSW2 = 0xAF;
} // namespace sw

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
} // namespace chaining

namespace iso7816
{
constexpr unsigned char LeResponse = 0x00;
} // namespace iso7816

#ifndef SWIG
namespace ins
{
namespace host
{
constexpr unsigned char AuthenticateHost = 0xA4;
} // namespace host

namespace key
{
constexpr unsigned char GetKeyEntry            = 0x64;
constexpr unsigned char ChangeKeyEntry         = 0xC1;
constexpr unsigned char DisableKeyEntryOffline = 0xD8;
constexpr unsigned char EncipherKeyEntry       = 0xE1;
constexpr unsigned char DumpSecretKey          = 0xD6;
} // namespace key

namespace kuc
{
constexpr unsigned char GetEntry    = 0x6C;
constexpr unsigned char ChangeEntry = 0xCC;
} // namespace kuc

namespace offline
{
constexpr unsigned char ActivateKey  = 0x01;
constexpr unsigned char DecipherData = 0x0D;
constexpr unsigned char EncipherData = 0x0E;
} // namespace offline

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
} // namespace pki

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
} // namespace emv

} // namespace ins
#endif

} // namespace sam
} // namespace logicalaccess

#endif /* LOGICALACCESS_SAMTYPES_HPP */