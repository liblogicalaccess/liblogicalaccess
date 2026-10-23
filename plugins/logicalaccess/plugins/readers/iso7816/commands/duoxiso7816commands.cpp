#include <logicalaccess/plugins/readers/iso7816/commands/duoxiso7816commands.hpp>

#include <logicalaccess/plugins/cards/desfire/desfirechip.hpp>
#include <logicalaccess/plugins/cards/desfire/desfireev2crypto.hpp>
#include <logicalaccess/plugins/cards/duox/duoxcrypto.hpp>
#include <logicalaccess/plugins/crypto/lla_random.hpp>
#include <logicalaccess/plugins/crypto/aes_helper.hpp>

// Remove later
#include <iomanip>
#include <sstream>
#include <logicalaccess/plugins/crypto/x509V3Certificate.hpp>
#include <limits>

// Move this inside duoxcrypto later
#include <algorithm>
#include <array>

#include <logicalaccess/plugins/cards/desfire/transactionsignaturecrypto.hpp>

namespace logicalaccess
{

namespace
{

// ---------------------------------------
// DUOX EV2 authentication
// ---------------------------------------

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

constexpr unsigned char TMAC_FILE_OPTION_TMI_EXCLUSION = 0x40;

constexpr unsigned char TMAC_KEY_OPTION_NONE   = 0x00;
constexpr unsigned char TMAC_KEY_OPTION_AES128 = 0x02;
constexpr unsigned char TMAC_KEY_OPTION_AES256 = 0x03;

constexpr unsigned char TMAC_KEY_OPTION_SIGNATURE = 0x80;

/**
 * \brief Clear and release sensitive byte data.
 */
void clearSensitiveBytes(ByteVector &data) // TODO replace it with SecureZero later
{
    std::fill(data.begin(), data.end(), static_cast<std::uint8_t>(0x00));
    data.clear();
}

/**
 * \brief Reset the EV2 authentication state after failure.
 * 
 * Authentication failures must never leave partially established secure messaging state in the crypto object.
 */
void resetDUOXEV2AuthenticationState(const std::shared_ptr<DESFireEV2Crypto> &crypto)
{
    if (!crypto)
        return;

    clearSensitiveBytes(crypto->d_sessionKey);
    clearSensitiveBytes(crypto->d_macSessionKey);

    crypto->setTI(ByteVector());
    crypto->setCmdCtr(0);
    crypto->d_auth_method = CryptoMethod::CM_LEGACY;
}

void validateAESKeyLength(DESFireKeyType keyType, std::size_t length)
{
    const std::size_t expectedLength = keyType == DF_KEY_AES ? DESFIRE_AES_KEY_SIZE : DESFIRE_AES256_KEY_SIZE;

    EXCEPTION_ASSERT_WITH_LOG(length == expectedLength,
        LibLogicalAccessException, "DUOX EV2 authentication key length does not match its key type.");
}

void printInfo(const std::string &message)
{
    std::cout << "[INFO] " << message << '\n';
}

static std::string hexDump(const ByteVector &data)
{
    std::ostringstream oss;
    for (std::size_t i = 0; i < data.size(); ++i)
    {
        if (i != 0)
            oss << ' ';
        oss << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << static_cast<unsigned int>(data[i]);
    }
    return oss.str();
}

static void debugBytes(const std::string &label, const ByteVector &data)
{
    printInfo(label + " [" + std::to_string(data.size()) + " bytes] : " + hexDump(data));
}

// ---------------------------------------
// ECDSA certificate verification
// ---------------------------------------

using EVP_PKEY_ptr = std::unique_ptr<EVP_PKEY, decltype(&EVP_PKEY_free)>;
using EVP_MD_CTX_ptr = std::unique_ptr<EVP_MD_CTX, decltype(&EVP_MD_CTX_free)>;

void verifySignatureWithCertificate(const ByteVector &certificateDER, const ByteVector &message,
                                    const ECDSASignature &signature)
{
    logicalaccess::X509V3Certificate certificate(certificateDER);

    EVP_PKEY_ptr certificateKey(certificate.getEVPPublicKey(), &EVP_PKEY_free);

    EXCEPTION_ASSERT_WITH_LOG(certificateKey != nullptr,
        LibLogicalAccessException, "Cert.A does not contain a public key.");

    EXCEPTION_ASSERT_WITH_LOG(EVP_PKEY_base_id(certificateKey.get()) == EVP_PKEY_EC,
        LibLogicalAccessException, "Cert.A public key is not an EC key.");

    // Convert DUOX r/s representation back to the DER signature representation expected by OpenSSL
    const ByteVector derSignature = encodeECDSASignatureDER(signature);

    EXCEPTION_ASSERT_WITH_LOG(!derSignature.empty(),
        LibLogicalAccessException, "Unable to encode DUOX ECDSA signature.");

    // Verify ECDSA-SHA256(message, Cert.A public key)
    // signECDSA() hashes the message internally so verification must use SHA-256 as well
    EVP_MD_CTX_ptr context(EVP_MD_CTX_new(), &EVP_MD_CTX_free);

    EXCEPTION_ASSERT_WITH_LOG(context != nullptr,
        LibLogicalAccessException, "Unable to allocate EVP_MD_CTX.");

    EXCEPTION_ASSERT_WITH_LOG(EVP_DigestVerifyInit(context.get(), nullptr, EVP_sha256(), nullptr, certificateKey.get()) == 1,
        LibLogicalAccessException, "EVP_DigestVerifyInit failed for Cert.A public key.");

    EXCEPTION_ASSERT_WITH_LOG(EVP_DigestVerifyUpdate(context.get(), message.data(), message.size()) == 1,
        LibLogicalAccessException, "EVP_DigestVerifyUpdate failed.");

    const int result = EVP_DigestVerifyFinal(context.get(), derSignature.data(), derSignature.size());

    EXCEPTION_ASSERT_WITH_LOG(result == 1,
        LibLogicalAccessException, "DUOX Sig.A verification failed using the public key contained in Cert.A.");

    printInfo("DUOX Sig.A verified with Cert.A public key.");
}

// ---------------------------------------
// Instructions and protocol constants
// ---------------------------------------

constexpr std::uint8_t INS_GET_KEY_SETTINGS   = 0x45;
constexpr std::uint8_t INS_MANAGE_KEY_PAIR    = 0x46;
constexpr std::uint8_t INS_EXPORT_KEY         = 0x47;
constexpr std::uint8_t INS_MANAGE_CA_ROOT_KEY = 0x48;
constexpr std::uint8_t INS_CHANGE_KEY         = 0xC4;
constexpr std::uint8_t INS_CHANGE_KEY_EV2     = 0xC6;

constexpr std::uint8_t INS_AUTHENTICATE_EV2_NON_FIRST = 0x77;

// ExportKey options
constexpr std::uint8_t EXPORT_KEY_OPTION_CA_ROOT_KEY = 0x01;

// DUOX key-number limits
constexpr std::uint8_t PICC_LEVEL_AID  = 0x00;
constexpr std::uint8_t MAX_PICC_KEY_NO = 0x01;
constexpr std::uint8_t MAX_KEY_NO      = 0x04;

// ---------------------------------------
// ECC key sizes
// ---------------------------------------

constexpr std::size_t ECC_PRIVATE_KEY_SIZE = 32;

/*
 * P-256 uncompressed public key :
 *   0x04 || X || Y
 */
constexpr std::size_t ECC_PUBLIC_KEY_SIZE = 65;

constexpr std::uint8_t ECC_UNCOMPRESSED_POINT_PREFIX = 0x04;

// ---------------------------------------
// DUOX communication modes
// ---------------------------------------

/**
 * \brief Check whether a DUOX communication mode is supported.
 *
 * \return true for Plain, MAC, or Full communication modes; otherwise false.
 */
constexpr bool isValidCommunicationMode(DUOXCommunicationMode mode)
{
    switch (mode)
    {
    case DUOXCommunicationMode::Plain:
    case DUOXCommunicationMode::MAC:
    case DUOXCommunicationMode::Full: return true;

    default: return false;
    }
}

// ---------------------------------------
// Access rights and key policy
// ---------------------------------------

/*
 * WriteAccess :
 *   bits 0-3 : WriteAR
 *   bits 4-5 : communication mode
 *   bits 6-7 : RFU
 */
constexpr std::uint8_t ACCESS_COM_MODE_MASK = 0x30;
constexpr std::uint8_t ACCESS_RFU_MASK      = 0xC0;

// AccessRights bits 14 and 15 are RFU
constexpr std::uint16_t ACCESS_RIGHTS_VALID_MASK = 0x3FFF;

/*
 * CRLFile :
 *
 *   bit 7    : CRL enabled
 *   bits 5-6 : RFU
 *   bits 0-4 : CRL File number when enabled, otherwise zero
 */
constexpr std::uint8_t CRL_FILE_ENABLED_MASK = 0x80;
constexpr std::uint8_t CRL_FILE_NUMBER_MASK  = 0x1F;
constexpr std::uint8_t CRL_FILE_RFU_MASK     = 0x60;
constexpr std::uint32_t AID_3_BYTE_MASK      = 0x00FFFFFF;

constexpr std::uint16_t KEY_POLICY_VALID_MASK =
    static_cast<std::uint16_t>((1u << 4) | // CryptoRequest
                               (1u << 5) | // SecureDynamicMessaging
                               (1u << 6) | // TransactionSignature
                               (1u << 7) | // MutualAuthentication
                               (1u << 8) | // UnilateralAuthentication
                               (1u << 15)  // FreezeKeyUsageCounter
    );

// ---------------------------------------
// Integer encoding helpers
// ---------------------------------------

/**
 * \brief Append an unsigned 16-bit integer in little-endian order.
 */
void appendUInt16LE(ByteVector &buffer, std::uint16_t value)
{
    buffer.push_back(static_cast<std::uint8_t>(value & 0xFF));
    buffer.push_back(static_cast<std::uint8_t>((value >> 8) & 0xFF));
}

/**
 * \brief Append an unsigned 24-bit integer in little-endian order.
 */
void appendUInt24LE(ByteVector &buffer, std::uint32_t value)
{
    buffer.push_back(static_cast<std::uint8_t>(value & 0xFFU));
    buffer.push_back(static_cast<std::uint8_t>((value >> 8U) & 0xFFU));
    buffer.push_back(static_cast<std::uint8_t>((value >> 16U) & 0xFFU));
}

/**
 * \brief Append an unsigned 32-bit integer in little-endian order.
 */
void appendUInt32LE(ByteVector &buffer, std::uint32_t value)
{
    buffer.push_back(static_cast<std::uint8_t>(value & 0xFF));
    buffer.push_back(static_cast<std::uint8_t>((value >> 8) & 0xFF));
    buffer.push_back(static_cast<std::uint8_t>((value >> 16) & 0xFF));
    buffer.push_back(static_cast<std::uint8_t>((value >> 24) & 0xFF));
}

std::string byteToHex(std::uint8_t value)
{
    std::ostringstream stream;
    stream << "0x" << std::hex << std::uppercase << static_cast<unsigned int>(value);
    return stream.str();
}

// ---------------------------------------
// ISO7816 / DUOX status words
// ---------------------------------------

struct DUOXExpectedStatus
{
    std::uint8_t sw1;
    std::uint8_t sw2;
};

constexpr DUOXExpectedStatus SUCCESS      = {0x90, 0x00};
constexpr DUOXExpectedStatus DUOX_SUCCESS = {0x91, 0x00};
constexpr DUOXExpectedStatus DUOX_ADDITIONAL_FRAME = {0x91, DF_INS_ADDITIONAL_FRAME};

constexpr DUOXExpectedStatus DUOX_FILE_LIMITED_FUNCTIONALITY = {0x62, 0x83};

bool isSelectFileSuccess(const ISO7816Response &response, DUOXExpectedStatus expected)
{
    return response.getSW1() == expected.sw1 && response.getSW2() == expected.sw2;
}

// For now any non-zero status on 0x9100 is considered failure
void checkSuccess(const ISO7816Response &response, DUOXExpectedStatus expected, const char *caller)
{
    EXCEPTION_ASSERT_WITH_LOG(response.getSW1() == expected.sw1 && response.getSW2() == expected.sw2,
        LibLogicalAccessException,
        std::string(caller) + " failed with status word " +
            byteToHex(response.getSW1()) + " " + byteToHex(response.getSW2()) +
            ", expected " + byteToHex(expected.sw1) + " " + byteToHex(expected.sw2) + ".");
}

// Might be better than checkSuccess if only want to check SW2 like DUOX commands do. But at transmit,
// we get transmission response (SW1=0x91 or another code in very rare cases of error) and DUOX response (SW1)
void checkSW2(const ISO7816Response &response, std::uint8_t expectedSW2, const char *caller)
{
    EXCEPTION_ASSERT_WITH_LOG(response.getSW2() == expectedSW2,
        LibLogicalAccessException,
        std::string(caller) + " failed with SW2 " + byteToHex(response.getSW2()) + ", expected " +
        byteToHex(expectedSW2) + ".");
}

// ---------------------------------------
// ISOSelectFile response validation
// ---------------------------------------

/**
 * \brief Validate the DUOX encrypted UID FCI format
 *
 * Expected structure : 6F 22 85 20 <32-byte encrypted UID>
 */
bool isValidEncryptedUIDFCI(const ByteVector &data)
{
    constexpr std::uint8_t FCI_TAG                  = 0x6F;
    constexpr std::uint8_t AUTH_DATA_TAG            = 0x85;
    constexpr std::uint8_t ENCRYPTED_UID_FCI_LENGTH = 0x22;
    constexpr std::uint8_t ENCRYPTED_UID_LENGTH     = 0x20;
    constexpr std::size_t RESPONSE_LENGTH           = 36;

    if (data.size() != RESPONSE_LENGTH)
        return false;

    return data[0] == FCI_TAG && data[1] == ENCRYPTED_UID_FCI_LENGTH &&
           data[2] == AUTH_DATA_TAG && data[3] == ENCRYPTED_UID_LENGTH;
}

/**
 * \brief Validate the DUOX ephemeral public key FCI format
 *
 * Expected structure : 6F 43 85 41 04 <32-byte X coordinate> <32-byte Y coordinate>
 */
bool isValidEphemeralPublicKeyFCI(const ByteVector &data)
{
    constexpr std::uint8_t FCI_TAG            = 0x6F;
    constexpr std::uint8_t AUTH_DATA_TAG      = 0x85;
    constexpr std::uint8_t FCI_LENGTH         = 0x43;
    constexpr std::uint8_t PUBLIC_KEY_LENGTH  = 0x41;
    constexpr std::size_t RESPONSE_LENGTH     = 69;

    if (data.size() != RESPONSE_LENGTH)
        return false;

    return data[0] == FCI_TAG && data[1] == FCI_LENGTH &&
           data[2] == AUTH_DATA_TAG && data[3] == PUBLIC_KEY_LENGTH &&
           data[4] == ECC_UNCOMPRESSED_POINT_PREFIX;
}

/**
 * \brief Validate ISOSelectFile response data.
 *
 * ISO SELECT FILE may return :
 *   - No data
 *   - Application defined FCI
 *   - DUOX encrypted UID FCI
 *   - DUOX ephemeral public-key FCI
 *
 * Application defined FCI is deliberately NOT interpreted here.
 * Content is application-specific and mustn't be rejected because it doesn't match one of the authentication formats.
 */
void validateFileResponseData(const ByteVector &data)
{
    // TODO : For now this function does nothing but we might re-use it later when inspecting returned structures

    if (data.empty())
        return;

    if (isValidEncryptedUIDFCI(data))
        return;

    if (isValidEphemeralPublicKeyFCI(data))
        return;

    // Anything else is intentionally accepted in order to accept arbitrary application-defined FCI
}

// ---------------------------------------
// Authentication method helpers
// ---------------------------------------

constexpr bool isSupportedAuthMethod(AuthMethod authMethod)
{
    switch (authMethod)
    {
    case AuthMethod::AsymmetricMutualWithCertificate:
    case AuthMethod::AsymmetricMutualWithoutCertificate:
    case AuthMethod::ReaderUnilateralWithCertificate:
    case AuthMethod::ReaderUnilateralWithoutCertificate:
        return true;

    default: return false;
    }
}

constexpr bool usesCertificate(AuthMethod authMethod)
{
    switch (authMethod)
    {
    case AuthMethod::AsymmetricMutualWithCertificate:
    case AuthMethod::ReaderUnilateralWithCertificate:
        return true;

    case AuthMethod::AsymmetricMutualWithoutCertificate:
    case AuthMethod::ReaderUnilateralWithoutCertificate:
        return false;

    default:
        return false;
    }
}

constexpr bool isMutualAuthentication(AuthMethod authMethod)
{
    switch (authMethod)
    {
    case AuthMethod::AsymmetricMutualWithCertificate:
    case AuthMethod::AsymmetricMutualWithoutCertificate:
        return true;

    case AuthMethod::ReaderUnilateralWithCertificate:
    case AuthMethod::ReaderUnilateralWithoutCertificate:
        return false;

    default:
        return false;
    }
}

// ---------------------------------------
// DER encoding helpers
// ---------------------------------------

// For now, don't re-use TLV class for ISOGeneralAuthenticateFinal
std::size_t parseDERLength(const ByteVector &data, std::size_t &offset)
{
    EXCEPTION_ASSERT_WITH_LOG(offset < data.size(),
        LibLogicalAccessException, "Invalid DER length : length field is missing.");

    const std::uint8_t firstByte = data[offset++];

    // DER short-form length : 0x00...0x7F
    if ((firstByte & 0x80U) == 0)
        return firstByte;

    const std::uint8_t lengthOctets = firstByte & 0x7FU;

    EXCEPTION_ASSERT_WITH_LOG(lengthOctets != 0,
        LibLogicalAccessException, "Invalid DER length : indefinite length encoding is forbidden.");

    EXCEPTION_ASSERT_WITH_LOG(lengthOctets <= sizeof(std::size_t),
        LibLogicalAccessException, "Invalid DER length : length field is too large.");

    EXCEPTION_ASSERT_WITH_LOG(offset + lengthOctets <= data.size(),
        LibLogicalAccessException, "Invalid DER length : truncated length field.");

    // DER requires the minimum possible length encoding
    EXCEPTION_ASSERT_WITH_LOG(data[offset] != 0x00,
        LibLogicalAccessException, "Invalid DER length: leading zero is not permitted.");

    std::size_t length = 0;
    for (std::uint8_t i = 0; i < lengthOctets; ++i)
    {
        EXCEPTION_ASSERT_WITH_LOG(length <= ((std::numeric_limits<std::size_t>::max)() >> 8),
            LibLogicalAccessException, "Invalid DER length : integer overflow.");

        length = (length << 8) | data[offset++];
    }

    EXCEPTION_ASSERT_WITH_LOG(length >= 0x80,
        LibLogicalAccessException, "Invalid DER length : non-minimal long-form encoding.");

    EXCEPTION_ASSERT_WITH_LOG(length <= data.size() - offset,
        LibLogicalAccessException, "Invalid DER length : declared value exceeds available data.");

    return length;
}

// For now, don't re-use TLV class for ISOGeneralAuthenticateFinal
ByteVector encodeDERLength(std::size_t length)
{
    if (length < 0x80)
        return {static_cast<std::uint8_t>(length)};

    if (length <= 0xFF)
        return {0x81, static_cast<std::uint8_t>(length)};

    if (length <= 0xFFFF)
        return {0x82, static_cast<std::uint8_t>((length >> 8) & 0xFF), static_cast<std::uint8_t>(length & 0xFF)};

    THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "DER length exceeds the supported DUOX range.");
}

// For now, don't re-use TLV class for ISOGeneralAuthenticateFinal
void appendDERTLV(ByteVector &output, std::uint8_t tag, const ByteVector &value)
{
    output.push_back(tag);
    const ByteVector encodedLength = encodeDERLength(value.size());
    output.insert(output.end(), encodedLength.begin(), encodedLength.end());
    output.insert(output.end(), value.begin(), value.end());
}

/**
 * \brief Returns the TMKeyOption value corresponding to a DUOX Transaction MAC key.
 *
 * Validates that the key is non-null and that its configured AES key type matches its data length.
 * DUOX supports AES-128 (16 bytes) and AES-256 (32 bytes) for the Transaction MAC key.
 *
 * \param key Transaction MAC key.
 */
unsigned char getTransactionMACKeyOption(const std::shared_ptr<DESFireKey> &key)
{
    EXCEPTION_ASSERT_WITH_LOG(key != nullptr, LibLogicalAccessException, "A Transaction MAC key is required.");

    const auto keyData = key->getData();
    const auto keySize = keyData.size();

    EXCEPTION_ASSERT_WITH_LOG((keySize == 16 && key->getKeyType() == DF_KEY_AES) ||
                              (keySize == 32 && key->getKeyType() == DF_KEY_AES256),
        LibLogicalAccessException, "Transaction MAC key type and key length are inconsistent.");

    EXCEPTION_ASSERT_WITH_LOG(keySize == 16 || keySize == 32,
        LibLogicalAccessException, "DUOX Transaction MAC key must contain 16 or 32 bytes.");

    return keySize == 16 ? TMAC_KEY_OPTION_AES128 : TMAC_KEY_OPTION_AES256;
}

} // namespace

// DUOXCommands and DESFireEV3ISO7816Commands both derive from DESFireEV3Commands
// The inheritance is intentionally non-virtual for now
// The explicit forwarding below selects the ISO7816 implementation for all
// DESFire commands and resolves the resulting diamond/ambiguity

DUOXISO7816Commands::DUOXISO7816Commands()
    : DESFireEV3ISO7816Commands(CMD_DUOXISO7816)
{
    // DUOXCommands provides the DUOX command abstraction while
    // DESFireEV3ISO7816Commands provides the ISO7816 implementation
}

// ---------------------------------------
// Common / transport
// ---------------------------------------
std::shared_ptr<Chip> DUOXISO7816Commands::getChip() const
{
    return DESFireEV3ISO7816Commands::getChip();
}

std::shared_ptr<ReaderCardAdapter> DUOXISO7816Commands::getReaderCardAdapter() const
{
    return DESFireEV3ISO7816Commands::getReaderCardAdapter();
}

ISO7816Response DUOXISO7816Commands::transmitDUOX(std::uint8_t cmd, const ByteVector &params, const ByteVector &data,
    DUOXCommunicationMode commMode)
{
    switch (commMode)
    {
    case DUOXCommunicationMode::Plain:
    case DUOXCommunicationMode::MAC:
    {
        ByteVector command;
        command.reserve(params.size() + data.size());
        command.insert(command.end(), params.begin(), params.end());
        command.insert(command.end(), data.begin(), data.end());

        // Differentiate Plain and MAC transmission after shared command preparation
        if (commMode == DUOXCommunicationMode::Plain)
            return transmit_plain(cmd, command);

        return transmit(cmd, command);
    }

    case DUOXCommunicationMode::Full: return transmit_full(cmd, data, params);

    default:
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Invalid DUOX communication mode.");
    }
}

// ---------------------------------------
// DUOX
// ---------------------------------------

ByteVector DUOXISO7816Commands::manageKeyPair(std::uint8_t keyNo, DUOXManageKeyPairOption option,
                                              CurveID curveId, std::uint16_t keyPolicy,
                                              std::uint8_t writeAccess, std::uint32_t kucLimit,
                                              const ByteVector &privateKey, DUOXCommunicationMode commMode)
{
    const auto chip = getDESFireChip();

    EXCEPTION_ASSERT_WITH_LOG(chip != nullptr,
        LibLogicalAccessException, "DUOX ManageKeyPair requires an initialized DESFire chip.");

    const auto crypto = chip->getCrypto();

    EXCEPTION_ASSERT_WITH_LOG(crypto != nullptr,
        LibLogicalAccessException, "DUOX ManageKeyPair requires an initialized DESFire crypto context.");

    const bool piccLevel = crypto->d_currentAid == PICC_LEVEL_AID;

    const std::uint8_t maximumKeyNo = piccLevel ? MAX_PICC_KEY_NO : MAX_KEY_NO;

    EXCEPTION_ASSERT_WITH_LOG(keyNo <= maximumKeyNo,
        LibLogicalAccessException,
        "Invalid DUOX ECC private key number " + byteToHex(keyNo) +
            ". Maximum allowed key number is " + byteToHex(maximumKeyNo) +
            (piccLevel ? " at PICC level." : " at application level."));

    switch (option)
    {
    case DUOXManageKeyPairOption::GenerateKeyPair:
        EXCEPTION_ASSERT_WITH_LOG(privateKey.empty(),
            LibLogicalAccessException, "A private key must not be supplied for DUOX ManageKeyPair key-pair generation.");
        break;

    case DUOXManageKeyPairOption::ImportPrivateKey:
        EXCEPTION_ASSERT_WITH_LOG(privateKey.size() == ECC_PRIVATE_KEY_SIZE,
            LibLogicalAccessException, "DUOX ManageKeyPair private key import requires exactly 32 bytes.");
        break;

    case DUOXManageKeyPairOption::UpdateMetadata:
        EXCEPTION_ASSERT_WITH_LOG(privateKey.empty(),
            LibLogicalAccessException, "A private key must not be supplied for DUOX ManageKeyPair metadata update.");
        break;

    default:
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Invalid DUOX ManageKeyPair option.");
    }

    switch (curveId)
    {
    case CurveID::NIST_P256:
    case CurveID::BRAINPOOL_P256R1: break;
    default:
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
            "Unsupported DUOX CurveID " + byteToHex(static_cast<std::uint8_t>(curveId)) + ".");
    }

    EXCEPTION_ASSERT_WITH_LOG((keyPolicy & ~KEY_POLICY_VALID_MASK) == 0,
        LibLogicalAccessException, "Invalid DUOX KeyPolicy : reserved bits are set.");

    EXCEPTION_ASSERT_WITH_LOG((writeAccess & ACCESS_RFU_MASK) == 0,
        LibLogicalAccessException, "Invalid DUOX WriteAccess : reserved bits 6 and 7 must be zero.");

    const auto writeCommunicationMode = static_cast<DUOXCommunicationMode>((writeAccess & ACCESS_COM_MODE_MASK) >> 4);

    EXCEPTION_ASSERT_WITH_LOG(isValidCommunicationMode(writeCommunicationMode),
        LibLogicalAccessException, "Invalid DUOX WriteAccess communication mode.");
    
    constexpr std::size_t HEADER_PARAMETERS_SIZE = sizeof(std::uint8_t) +  // KeyNo
                                                   sizeof(std::uint8_t) +  // Option
                                                   sizeof(std::uint8_t) +  // CurveID
                                                   sizeof(std::uint16_t) + // KeyPolicy
                                                   sizeof(std::uint8_t) +  // WriteAccess
                                                   sizeof(std::uint32_t);  // KUCLimit

    ByteVector params;
    params.reserve(HEADER_PARAMETERS_SIZE);

    params.push_back(keyNo);
    params.push_back(static_cast<std::uint8_t>(option));
    params.push_back(static_cast<std::uint8_t>(curveId));
    appendUInt16LE(params, keyPolicy);
    params.push_back(writeAccess);
    appendUInt32LE(params, kucLimit);

    ByteVector command;
    if (option == DUOXManageKeyPairOption::ImportPrivateKey)
        command = privateKey;
    
    const auto response = transmitDUOX(INS_MANAGE_KEY_PAIR, params, command, commMode);

    checkSuccess(response, DUOX_SUCCESS, __func__);

    switch (option)
    {
    case DUOXManageKeyPairOption::GenerateKeyPair:
    {
        const auto &responseData = response.getData();

        EXCEPTION_ASSERT_WITH_LOG(!responseData.empty(), LibLogicalAccessException,
            "DUOX ManageKeyPair key generation succeeded but the card returned an empty public key.");

        EXCEPTION_ASSERT_WITH_LOG(responseData.front() == ECC_UNCOMPRESSED_POINT_PREFIX,
            LibLogicalAccessException, "DUOX ManageKeyPair key generation returned an invalid uncompressed public key.");

        return responseData;
    }

    case DUOXManageKeyPairOption::ImportPrivateKey:
    case DUOXManageKeyPairOption::UpdateMetadata:
        EXCEPTION_ASSERT_WITH_LOG(response.getData().empty(), LibLogicalAccessException,
            "DUOX ManageKeyPair returned unexpected response data for an operation that does not return a public key.");
        return {};

    default:
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Invalid DUOX ManageKeyPair option.");
    }
}

void DUOXISO7816Commands::manageCARootKey(
    std::uint8_t keyNo, CurveID curveId, std::uint16_t accessRights,
    std::uint8_t writeAccess, std::uint8_t readAccess, std::uint8_t crlFile,
    std::uint32_t crlFileAid, const ByteVector &publicKey, const ByteVector &issuer,
    DUOXCommunicationMode commMode)
{
    const auto chip = getDESFireChip();

    EXCEPTION_ASSERT_WITH_LOG(chip != nullptr,
        LibLogicalAccessException, "DUOX ManageCARootKey requires an initialized DESFire chip.");

    const auto crypto = chip->getCrypto();

    EXCEPTION_ASSERT_WITH_LOG(crypto != nullptr,
        LibLogicalAccessException, "DUOX ManageCARootKey requires an initialized DESFire crypto context.");

    const bool piccLevel = crypto->d_currentAid == PICC_LEVEL_AID;
    const std::uint8_t maximumKeyNo = piccLevel ? MAX_PICC_KEY_NO : MAX_KEY_NO;

    EXCEPTION_ASSERT_WITH_LOG(keyNo <= maximumKeyNo, LibLogicalAccessException,
        "Invalid DUOX CA Root Key number " + byteToHex(keyNo) +
            ". Maximum allowed key number is " + byteToHex(maximumKeyNo) +
            (piccLevel ? " at PICC level." : " at application level."));

    // CurveID currently supports only the two curves defined by DUOX.
    switch (curveId)
    {
    case CurveID::NIST_P256:
    case CurveID::BRAINPOOL_P256R1: break;

    default:
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
            "Unsupported DUOX CurveID " + byteToHex(static_cast<std::uint8_t>(curveId)) + ".");
    }

    EXCEPTION_ASSERT_WITH_LOG((accessRights & ~ACCESS_RIGHTS_VALID_MASK) == 0,
        LibLogicalAccessException, "Invalid DUOX CA Root Key AccessRights : reserved bits 14 and 15 must be zero.");

    EXCEPTION_ASSERT_WITH_LOG((writeAccess & ACCESS_RFU_MASK) == 0,
        LibLogicalAccessException, "Invalid DUOX WriteAccess : reserved bits 6 and 7 must be zero.");

    EXCEPTION_ASSERT_WITH_LOG((readAccess & ACCESS_RFU_MASK) == 0,
        LibLogicalAccessException, "Invalid DUOX ReadAccess : reserved bits 6 and 7 must be zero.");

    const auto writeCommunicationMode = static_cast<DUOXCommunicationMode>((writeAccess & ACCESS_COM_MODE_MASK) >> 4);

    EXCEPTION_ASSERT_WITH_LOG(isValidCommunicationMode(writeCommunicationMode),
        LibLogicalAccessException, "Invalid DUOX WriteAccess communication mode.");

    const auto readCommunicationMode = static_cast<DUOXCommunicationMode>((readAccess & ACCESS_COM_MODE_MASK) >> 4);

    EXCEPTION_ASSERT_WITH_LOG(isValidCommunicationMode(readCommunicationMode),
        LibLogicalAccessException, "Invalid DUOX ReadAccess communication mode.");

    EXCEPTION_ASSERT_WITH_LOG((crlFile & CRL_FILE_RFU_MASK) == 0,
        LibLogicalAccessException, "Invalid DUOX CRLFile : reserved bits 5 and 6 must be zero.");

    const bool crlEnabled = (crlFile & CRL_FILE_ENABLED_MASK) != 0;

    if (crlEnabled) // CRLFileAID is represented by exactly three bytes on the wire, LSB first
    {
        EXCEPTION_ASSERT_WITH_LOG(crlFileAid <= AID_3_BYTE_MASK,
            LibLogicalAccessException, "Invalid DUOX CRLFileAID : only 3-byte AIDs are supported.");
    }
    else
    {
        EXCEPTION_ASSERT_WITH_LOG(crlFile == 0x00, LibLogicalAccessException,
            "Invalid DUOX CRLFile : file number must be zero when certificate revocation is disabled.");

        EXCEPTION_ASSERT_WITH_LOG(crlFileAid == 0x000000, LibLogicalAccessException,
            "Invalid DUOX CRLFileAID : AID must be 0x000000 when certificate revocation is disabled.");
    }

    EXCEPTION_ASSERT_WITH_LOG(publicKey.size() == ECC_PUBLIC_KEY_SIZE, LibLogicalAccessException,
        "DUOX CA Root Key requires exactly 65 bytes for an uncompressed ECC public key (0x04 || X || Y).");

    EXCEPTION_ASSERT_WITH_LOG(publicKey.front() == ECC_UNCOMPRESSED_POINT_PREFIX, LibLogicalAccessException,
        "Invalid DUOX CA Root Key public key : uncompressed ECC public keys must start with 0x04.");

    /*
     * IssuerLen is one byte on the wire.
     * An empty issuer means that no trusted issuer check is requested and results in IssuerLen == 0
     */
    EXCEPTION_ASSERT_WITH_LOG(issuer.size() <= (std::numeric_limits<std::uint8_t>::max)(),
        LibLogicalAccessException, "DUOX CA Root Key issuer name is too long : maximum length is 255 bytes.");

    const auto issuerLength = static_cast<std::uint8_t>(issuer.size());

    constexpr std::size_t CRL_FILE_AID_SIZE = 3;

    constexpr std::size_t HEADER_PARAMETERS_SIZE = sizeof(std::uint8_t) +  // KeyNo
                                                   sizeof(std::uint8_t) +  // CurveID
                                                   sizeof(std::uint16_t) + // AccessRights
                                                   sizeof(std::uint8_t) +  // WriteAccess
                                                   sizeof(std::uint8_t) +  // ReadAccess
                                                   sizeof(std::uint8_t) +  // CRLFile
                                                   CRL_FILE_AID_SIZE;      // CRLFileAID

    constexpr std::size_t ISSUER_LENGTH_SIZE = sizeof(std::uint8_t);
    const std::size_t DATA_PARAMETERS_SIZE = ECC_PUBLIC_KEY_SIZE + ISSUER_LENGTH_SIZE + issuer.size();

    ByteVector params;
    params.reserve(HEADER_PARAMETERS_SIZE);

    params.push_back(keyNo);
    params.push_back(static_cast<std::uint8_t>(curveId));
    appendUInt16LE(params, accessRights);
    params.push_back(writeAccess);
    params.push_back(readAccess);
    params.push_back(crlFile);
    appendUInt24LE(params, crlFileAid);

    EXCEPTION_ASSERT_WITH_LOG(params.size() == HEADER_PARAMETERS_SIZE,
        LibLogicalAccessException, "Internal DUOX ManageCARootKey error : invalid header parameter size.");

    ByteVector command;
    command.reserve(DATA_PARAMETERS_SIZE);

    command.insert(command.end(), publicKey.begin(), publicKey.end());
    command.push_back(issuerLength);
    if (!issuer.empty())
        command.insert(command.end(), issuer.begin(), issuer.end());

    EXCEPTION_ASSERT_WITH_LOG(command.size() == DATA_PARAMETERS_SIZE,
        LibLogicalAccessException, "Internal DUOX ManageCARootKey error : invalid command data parameter size.");

    const auto response = transmitDUOX(INS_MANAGE_CA_ROOT_KEY, params, command, commMode);

    checkSuccess(response, DUOX_SUCCESS, __func__);

    EXCEPTION_ASSERT_WITH_LOG(response.getData().empty(),
        LibLogicalAccessException, "DUOX ManageCARootKey returned unexpected response data.");
}

ByteVector DUOXISO7816Commands::exportKey(std::uint8_t keyNo, DUOXCommunicationMode commMode)
{
    const auto chip = getDESFireChip();

    EXCEPTION_ASSERT_WITH_LOG(chip != nullptr,
        LibLogicalAccessException, "DUOX ExportKey requires an initialized DESFire chip.");

    const auto crypto = chip->getCrypto();

    EXCEPTION_ASSERT_WITH_LOG(crypto != nullptr,
        LibLogicalAccessException, "DUOX ExportKey requires an initialized DESFire crypto context.");

    const bool piccLevel = crypto->d_currentAid == PICC_LEVEL_AID;
    const std::uint8_t maximumKeyNo = piccLevel ? MAX_PICC_KEY_NO : MAX_KEY_NO;

    EXCEPTION_ASSERT_WITH_LOG(keyNo <= maximumKeyNo,
        LibLogicalAccessException,
        "Invalid DUOX ExportKey number " + byteToHex(keyNo) + ". Maximum allowed key number is " + byteToHex(maximumKeyNo) +
            (piccLevel ? " at PICC level." : " at application level."));

    constexpr std::size_t EXPORT_KEY_PARAMETERS_SIZE = sizeof(std::uint8_t) + // Option
                                                       sizeof(std::uint8_t);  // KeyNo

    ByteVector params;
    params.reserve(EXPORT_KEY_PARAMETERS_SIZE);

    params.push_back(EXPORT_KEY_OPTION_CA_ROOT_KEY);
    params.push_back(keyNo);

    EXCEPTION_ASSERT_WITH_LOG(params.size() == EXPORT_KEY_PARAMETERS_SIZE,
        LibLogicalAccessException, "Internal DUOX ExportKey error : invalid command parameter size.");

    const auto response = transmitDUOX(INS_EXPORT_KEY, params, {}, commMode);

    checkSuccess(response, DUOX_SUCCESS, __func__);

    const auto &publicKey = response.getData();

    EXCEPTION_ASSERT_WITH_LOG(publicKey.size() == ECC_PUBLIC_KEY_SIZE,
        LibLogicalAccessException, "DUOX ExportKey returned an invalid public key size : expected 65 bytes, received " +
            std::to_string(publicKey.size()) + " bytes.");

    EXCEPTION_ASSERT_WITH_LOG(publicKey.front() == ECC_UNCOMPRESSED_POINT_PREFIX,
        LibLogicalAccessException, "DUOX ExportKey returned an invalid public key : uncompressed ECC public keys must start with 0x04.");

    return publicKey;
}

void DUOXISO7816Commands::authenticateEV2NonFirst(std::uint8_t keyno, std::shared_ptr<DESFireKey> currentKey)
{
    const auto chip = getDESFireChip();

    EXCEPTION_ASSERT_WITH_LOG(chip != nullptr,
        LibLogicalAccessException, "DUOX EV2 AuthenticateEV2NonFirst requires an initialized DESFire chip.");

    const auto crypto = chip->getCrypto();

    EXCEPTION_ASSERT_WITH_LOG(crypto != nullptr,
        LibLogicalAccessException, "DUOX EV2 AuthenticateEV2NonFirst requires an initialized DESFire crypto context.");

    // AuthenticateEV2NonFirst is valid for both the DUOX and standard EV2 secure-messaging modes.
    // Preserve the authentication mode established by the preceding authentication step.
    EXCEPTION_ASSERT_WITH_LOG(usesEV2Crypto(crypto->d_auth_method),
        LibLogicalAccessException, "DUOX EV2 AuthenticateEV2NonFirst requires an active EV2 authentication.");

    const auto duoxCrypto = std::dynamic_pointer_cast<DUOXCrypto>(crypto);

    EXCEPTION_ASSERT_WITH_LOG(duoxCrypto != nullptr, LibLogicalAccessException,
        "DUOX EV2 AuthenticateEV2NonFirst requires the existing DUOX implementation.");

    if (!currentKey)
        currentKey = duoxCrypto->getKey(0, keyno);

    EXCEPTION_ASSERT_WITH_LOG(currentKey != nullptr,
        LibLogicalAccessException, "DUOX EV2 AuthenticateEV2NonFirst requires an authentication key.");

    const DESFireKeyType keyType = currentKey->getKeyType();

    EXCEPTION_ASSERT_WITH_LOG(keyType == DF_KEY_AES || keyType == DF_KEY_AES256,
        LibLogicalAccessException, "DUOX EV2 AuthenticateEV2NonFirst only supports AES-128 and AES-256 keys.");

    validateAESKeyLength(keyType, currentKey->getLength());

    const std::size_t expectedKeyLength = keyType == DF_KEY_AES ? DESFIRE_AES_KEY_SIZE : DESFIRE_AES256_KEY_SIZE;

    auto key = std::make_shared<DESFireKey>(*currentKey);

    const ByteVector keyDiversification = getKeyInformations(key, keyno);

    ByteVector keyDiv;
    duoxCrypto->getKey(key, keyDiversification, keyDiv);

    EXCEPTION_ASSERT_WITH_LOG(keyDiv.size() == expectedKeyLength,
        LibLogicalAccessException, "DUOX EV2 diversified authentication key has an invalid length.");

    const ByteVector iv(AES_BLOCK_SIZE, 0x00);
    const ByteVector command = {keyno};

    ISO7816Response response = transmit_plain(INS_AUTHENTICATE_EV2_NON_FIRST, command);
    
    checkSuccess(response, DUOX_ADDITIONAL_FRAME, __func__);

    EXCEPTION_ASSERT_WITH_LOG(response.getData().size() == EV2_RANDOM_SIZE,
        LibLogicalAccessException, "AuthenticateEV2NonFirst Part 1 returned an invalid RndB length.");

    ByteVector rndB = AESHelper::AESDecrypt(response.getData(), keyDiv, iv);

    EXCEPTION_ASSERT_WITH_LOG(rndB.size() == EV2_RANDOM_SIZE,
        LibLogicalAccessException, "AuthenticateEV2NonFirst Part 1 produced an invalid RndB.");

    // Generate PCD challenge
    ByteVector rndA = RandomHelper::bytes(EV2_RANDOM_SIZE);

    EXCEPTION_ASSERT_WITH_LOG(rndA.size() == EV2_RANDOM_SIZE,
        LibLogicalAccessException, "Random generator returned an invalid RndA length.");

    // RndB' = RndB rotated left by one byte
    ByteVector rndBPrime = DUOXCrypto::rotateAuthenticationChallenge(rndB);

    // Part 2 data : RndA || RndB'
    ByteVector challenge;
    challenge.reserve(EV2_PART2_COMMAND_SIZE);
    challenge.insert(challenge.end(), rndA.begin(), rndA.end());
    challenge.insert(challenge.end(), rndBPrime.begin(), rndBPrime.end());

    EXCEPTION_ASSERT_WITH_LOG(challenge.size() == EV2_PART2_COMMAND_SIZE, LibLogicalAccessException,
        "Internal DUOX EV2 authentication error : Part 2 plaintext must contain exactly 32 bytes.");

    ByteVector encryptedChallenge = AESHelper::AESEncrypt(challenge, keyDiv, iv);

    EXCEPTION_ASSERT_WITH_LOG(encryptedChallenge.size() == EV2_PART2_COMMAND_SIZE,
        LibLogicalAccessException, "AuthenticateEV2NonFirst Part 2 challenge returned an invalid length.");

    response = transmit_plain(DF_INS_ADDITIONAL_FRAME, encryptedChallenge);

    checkSuccess(response, DUOX_SUCCESS, __func__);

    EXCEPTION_ASSERT_WITH_LOG(response.getData().size() == EV2_RANDOM_SIZE,
        LibLogicalAccessException, "AuthenticateEV2NonFirst Part 2 returned an invalid RndA' length.");

    ByteVector rndAPrime = AESHelper::AESDecrypt(response.getData(), keyDiv, iv);

    EXCEPTION_ASSERT_WITH_LOG(rndAPrime.size() == EV2_RANDOM_SIZE,
        LibLogicalAccessException, "AuthenticateEV2NonFirst returned an invalid RndA'.");

    // Verify RndA' = RndA rotated left by one byte
    ByteVector expectedRndAPrime = DUOXCrypto::rotateAuthenticationChallenge(rndA);

    EXCEPTION_ASSERT_WITH_LOG(rndAPrime == expectedRndAPrime,
        LibLogicalAccessException, "AuthenticateEV2NonFirst authentication failed : invalid RndA'.");

    // Establish the EV2 secure-messaging session
    duoxCrypto->generateSessionKey(keyDiv, rndA, rndB);
    duoxCrypto->d_currentKeyNo = keyno;
    duoxCrypto->setKey(duoxCrypto->d_currentAid, 0, keyno, key);

    // Clear sensitive authentication material
    clearSensitiveBytes(rndA);
    clearSensitiveBytes(rndB);
    clearSensitiveBytes(rndBPrime);
    clearSensitiveBytes(challenge);
    clearSensitiveBytes(encryptedChallenge);
    clearSensitiveBytes(rndAPrime);
    clearSensitiveBytes(expectedRndAPrime);
    clearSensitiveBytes(keyDiv);
}

std::uint32_t DUOXISO7816Commands::freeMem()
{
    constexpr std::size_t FREE_MEM_RESPONSE_SIZE = 3U;

    const auto response = transmit(DFEV1_INS_FREE_MEM);
    const auto &data    = response.getData();

    EXCEPTION_ASSERT_WITH_LOG(data.size() == FREE_MEM_RESPONSE_SIZE,
        LibLogicalAccessException, "DESFire FreeMem returned an invalid response length.");

    return static_cast<std::uint32_t>(data[0]) |
           (static_cast<std::uint32_t>(data[1]) << 8U) |
           (static_cast<std::uint32_t>(data[2]) << 16U);
}

DUOXKeySettings DUOXISO7816Commands::getKeySettings(DUOXKeySettingsOption option)
{
    constexpr std::size_t ECC_METADATA_ENTRY_SIZE = 13;
    constexpr std::size_t CA_ROOT_METADATA_ENTRY_SIZE = 10;
    constexpr std::uint8_t MAX_METADATA_ENTRIES = 0x05;
    constexpr std::uint8_t KEY_TYPE_MASK = 0xC0;
    constexpr std::uint8_t KEY_COUNT_MASK = 0x3F;
    constexpr std::uint8_t KEY_TYPE_NA = 0x00;
    constexpr std::uint8_t KEY_TYPE_RESERVED = 0x40;
    constexpr std::uint8_t KEY_TYPE_AES128 = 0x80;
    constexpr std::uint8_t KEY_TYPE_AES256 = 0xC0;

    EXCEPTION_ASSERT_WITH_LOG(
        option == DUOXKeySettingsOption::KeySettings ||
        option == DUOXKeySettingsOption::ECCPrivateKeyMetadata ||
        option == DUOXKeySettingsOption::CARootKeyMetadata,
        LibLogicalAccessException, "Invalid DUOX GetKeySettings option : " + byteToHex(static_cast<std::uint8_t>(option)) +
            ". Supported values are 0x00, 0x01 and 0x02.");

    const auto chip = getDESFireChip();

    EXCEPTION_ASSERT_WITH_LOG(chip != nullptr,
        LibLogicalAccessException, "DUOX GetKeySettings requires an initialized DESFire chip.");

    const auto crypto = chip->getCrypto();

    EXCEPTION_ASSERT_WITH_LOG(crypto != nullptr,
        LibLogicalAccessException, "DUOX GetKeySettings requires an initialized DESFire crypto context.");

    const bool piccLevel = crypto->d_currentAid == PICC_LEVEL_AID;

    ByteVector params;
    if (option != DUOXKeySettingsOption::KeySettings)
    {
        params.reserve(1);
        params.push_back(static_cast<std::uint8_t>(option));
    }

    const auto response = transmitDUOX(INS_GET_KEY_SETTINGS, params, {}, DUOXCommunicationMode::MAC);

    checkSuccess(response, DUOX_SUCCESS, __func__);

    const ByteVector &data = response.getData();

    DUOXKeySettings result;
    result.piccLevel = piccLevel;

    // TODO refactor later : add a switch per option. Just set code in sub-functions, don't optimize any further
    // Each "if" section should be contained in a proper sub function

    // ECC private key metadata
    if (option == DUOXKeySettingsOption::ECCPrivateKeyMetadata)
    {
        result.responseType = DUOXKeySettingsOption::ECCPrivateKeyMetadata;

        EXCEPTION_ASSERT_WITH_LOG(!data.empty(),
            LibLogicalAccessException, "DUOX GetKeySettings ECC private key metadata response is empty.");

        const std::uint8_t entryCount = data[0];

        EXCEPTION_ASSERT_WITH_LOG(entryCount <= MAX_METADATA_ENTRIES, LibLogicalAccessException,
            "Invalid DUOX ECC private key metadata entry count " + byteToHex(entryCount) +
                ". Maximum supported value is 0x05.");

        const std::size_t expectedSize = 1 + static_cast<std::size_t>(entryCount) * ECC_METADATA_ENTRY_SIZE;

        EXCEPTION_ASSERT_WITH_LOG(data.size() == expectedSize, LibLogicalAccessException,
            "Invalid DUOX ECC private key metadata response length : expected " +
            std::to_string(expectedSize) + " bytes, received " + std::to_string(data.size()) + " bytes.");

        result.eccPrivateKeys.reserve(entryCount);

        std::size_t offset = 1;
        for (std::uint8_t index = 0; index < entryCount; ++index)
        {
            DUOXECCPrivateKeyMetadata entry;

            entry.keyNo = data[offset++];
            const std::uint8_t rawCurveId = data[offset++];

            switch (static_cast<CurveID>(rawCurveId))
            {
            case CurveID::NIST_P256:
            case CurveID::BRAINPOOL_P256R1:
                entry.curveId = static_cast<CurveID>(rawCurveId);
                break;
            default:
                THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                    "Invalid DUOX ECC private key metadata CurveID " +
                    byteToHex(rawCurveId) + " at entry " + std::to_string(index) + ".");
            }

            entry.keyPolicy = static_cast<std::uint16_t>(data[offset]) | (static_cast<std::uint16_t>(data[offset + 1]) << 8);
            offset += 2;
            entry.writeAccess = data[offset++];
            entry.keyUsageCtrLimit =
                static_cast<std::uint32_t>(data[offset]) |
                (static_cast<std::uint32_t>(data[offset + 1]) << 8) |
                (static_cast<std::uint32_t>(data[offset + 2]) << 16) |
                (static_cast<std::uint32_t>(data[offset + 3]) << 24);

            offset += 4;

            entry.keyUsageCtr = static_cast<std::uint32_t>(data[offset]) |
                                (static_cast<std::uint32_t>(data[offset + 1]) << 8) |
                                (static_cast<std::uint32_t>(data[offset + 2]) << 16) |
                                (static_cast<std::uint32_t>(data[offset + 3]) << 24);

            offset += 4;

            EXCEPTION_ASSERT_WITH_LOG((entry.keyPolicy & ~KEY_POLICY_VALID_MASK) == 0, LibLogicalAccessException,
                "Invalid DUOX ECC private key metadata KeyPolicy at entry " +
                std::to_string(index) + ": reserved bits are set.");

            EXCEPTION_ASSERT_WITH_LOG((entry.writeAccess & ACCESS_RFU_MASK) == 0, LibLogicalAccessException,
                "Invalid DUOX ECC private key metadata WriteAccess at entry " +
                std::to_string(index) + ": reserved bits are set.");

            result.eccPrivateKeys.push_back(entry);
        }

        EXCEPTION_ASSERT_WITH_LOG(offset == data.size(),
            LibLogicalAccessException, "Internal DUOX GetKeySettings ECC metadata parser error : unexpected trailing data.");

        return result;
    }

    // CA Root Key metadata
    if (option == DUOXKeySettingsOption::CARootKeyMetadata)
    {
        result.responseType = DUOXKeySettingsOption::CARootKeyMetadata;

        EXCEPTION_ASSERT_WITH_LOG(!data.empty(),
            LibLogicalAccessException, "DUOX GetKeySettings CA Root Key metadata response is empty.");

        const std::uint8_t entryCount = data[0];

        EXCEPTION_ASSERT_WITH_LOG(entryCount <= MAX_METADATA_ENTRIES, LibLogicalAccessException,
            "Invalid DUOX CA Root Key metadata entry count " + byteToHex(entryCount) +
            ". Maximum supported value is 0x05.");

        const std::size_t expectedSize = 1 + static_cast<std::size_t>(entryCount) * CA_ROOT_METADATA_ENTRY_SIZE;

        EXCEPTION_ASSERT_WITH_LOG(data.size() == expectedSize, LibLogicalAccessException,
            "Invalid DUOX CA Root Key metadata response length : expected " +
            std::to_string(expectedSize) + " bytes, received " + std::to_string(data.size()) + " bytes.");

        result.caRootKeys.reserve(entryCount);

        std::size_t offset = 1;
        for (std::uint8_t index = 0; index < entryCount; ++index)
        {
            DUOXCARootKeyMetadata entry;

            entry.keyNo = data[offset++];

            const std::uint8_t rawCurveId = data[offset++];

            switch (static_cast<CurveID>(rawCurveId))
            {
            case CurveID::NIST_P256:
            case CurveID::BRAINPOOL_P256R1:
                entry.curveId = static_cast<CurveID>(rawCurveId);
                break;

            default:
                THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                    "Invalid DUOX CA Root Key metadata CurveID " + byteToHex(rawCurveId) + " at entry " +
                    std::to_string(index) + ".");
            }

            entry.accessRights = static_cast<std::uint16_t>(data[offset]) |
                                 (static_cast<std::uint16_t>(data[offset + 1]) << 8);

            offset += 2;

            entry.writeAccess = data[offset++];
            entry.readAccess = data[offset++];
            entry.crlFile = data[offset++];

            entry.crlFileAid = static_cast<std::uint32_t>(data[offset]) |
                               (static_cast<std::uint32_t>(data[offset + 1]) << 8) |
                               (static_cast<std::uint32_t>(data[offset + 2]) << 16);

            offset += 3;

            EXCEPTION_ASSERT_WITH_LOG((entry.accessRights & ~ACCESS_RIGHTS_VALID_MASK) == 0, LibLogicalAccessException,
                "Invalid DUOX CA Root Key metadata AccessRights at entry " +
                std::to_string(index) + ": reserved bits are set.");

            EXCEPTION_ASSERT_WITH_LOG((entry.writeAccess & ACCESS_RFU_MASK) == 0, LibLogicalAccessException,
                "Invalid DUOX CA Root Key metadata WriteAccess at entry " +
                std::to_string(index) + ": reserved bits are set.");

            EXCEPTION_ASSERT_WITH_LOG((entry.readAccess & ACCESS_RFU_MASK) == 0, LibLogicalAccessException,
                "Invalid DUOX CA Root Key metadata ReadAccess at entry " +
                std::to_string(index) + ": reserved bits are set.");

            EXCEPTION_ASSERT_WITH_LOG((entry.crlFile & CRL_FILE_RFU_MASK) == 0, LibLogicalAccessException,
                "Invalid DUOX CA Root Key metadata CRLFile at entry " +
                std::to_string(index) + ": reserved bits are set.");

            const bool crlEnabled = (entry.crlFile & CRL_FILE_ENABLED_MASK) != 0;

            if (!crlEnabled)
            {
                EXCEPTION_ASSERT_WITH_LOG(entry.crlFile == 0x00, LibLogicalAccessException,
                    "Invalid DUOX CA Root Key metadata CRLFile at entry " +
                    std::to_string(index) + ": file number must be zero when CRL is disabled.");

                EXCEPTION_ASSERT_WITH_LOG(entry.crlFileAid == 0x000000, LibLogicalAccessException,
                    "Invalid DUOX CA Root Key metadata CRLFileAID at entry " +
                    std::to_string(index) + ": AID must be zero when CRL is disabled.");
            }

            result.caRootKeys.push_back(entry);
        }

        EXCEPTION_ASSERT_WITH_LOG(offset == data.size(), LibLogicalAccessException,
            "Internal DUOX GetKeySettings CA Root Key metadata parser error : unexpected trailing data.");

        return result;
    }

    //Base KeySettings response
    result.responseType = DUOXKeySettingsOption::KeySettings;

    EXCEPTION_ASSERT_WITH_LOG(data.size() >= 2, LibLogicalAccessException,
        "DUOX GetKeySettings returned an incomplete key-settings response : at least 2 bytes are required.");

    if (piccLevel)
    {
        EXCEPTION_ASSERT_WITH_LOG(data.size() == 2, LibLogicalAccessException,
            "Invalid DUOX PICC GetKeySettings response length : expected exactly 2 bytes, received " +
            std::to_string(data.size()) + " bytes.");
    }
    else
    {
        EXCEPTION_ASSERT_WITH_LOG(data.size() == 2 || data.size() == 6, LibLogicalAccessException,
            "Invalid DUOX application GetKeySettings response length : expected 2 or 6 bytes, received " +
            std::to_string(data.size()) + " bytes.");
    }

    std::size_t offset = 0;

    result.keySettings = data[offset++];
    result.maxNoOfKeys = data[offset++];
    result.keyType = result.maxNoOfKeys & KEY_TYPE_MASK;
    result.numberOfKeys = result.maxNoOfKeys & KEY_COUNT_MASK;

    EXCEPTION_ASSERT_WITH_LOG(result.keyType != KEY_TYPE_RESERVED, LibLogicalAccessException,
        "Invalid DUOX GetKeySettings MaxNoOfKeys : reserved key-type encoding 01b was returned.");

    if (piccLevel)
    {
        EXCEPTION_ASSERT_WITH_LOG(result.numberOfKeys == 0x01, LibLogicalAccessException,
            "Invalid DUOX PICC GetKeySettings MaxNoOfKeys : bits 0-5 must be 0x01, received " +
            byteToHex(result.numberOfKeys) + ".");

        EXCEPTION_ASSERT_WITH_LOG(result.keyType == KEY_TYPE_AES128 || result.keyType == KEY_TYPE_AES256,
            LibLogicalAccessException, "Invalid DUOX PICC GetKeySettings key type : expected AES-128 or AES-256.");
    }
    else
    {
        if (result.numberOfKeys == 0x00)
        {
            EXCEPTION_ASSERT_WITH_LOG(result.keyType == KEY_TYPE_NA, LibLogicalAccessException,
                "Invalid DUOX application GetKeySettings response : an application containing no keys must report "
                "key type 00b.");
        }
        else
        {
            EXCEPTION_ASSERT_WITH_LOG(result.keyType == KEY_TYPE_AES128 || result.keyType == KEY_TYPE_AES256,
                LibLogicalAccessException,
                "Invalid DUOX application GetKeySettings key type : expected AES-128 or AES-256.");
        }
    }

    if (!piccLevel && data.size() == 6)
    {
        result.hasApplicationKeySetSettings = true;
        result.aksVersion = data[offset++];
        result.noKeySets = data[offset++];
        result.maxKeySize = data[offset++];
        result.appKeySetSettings = data[offset++];

        EXCEPTION_ASSERT_WITH_LOG(result.noKeySets >= 0x02 && result.noKeySets <= 0x10, LibLogicalAccessException,
            "Invalid DUOX GetKeySettings NoKeySets value " +
            byteToHex(result.noKeySets) + ": expected a value between 0x02 and 0x10.");

        EXCEPTION_ASSERT_WITH_LOG(result.maxKeySize == 0x10 || result.maxKeySize == 0x20, LibLogicalAccessException,
            "Invalid DUOX GetKeySettings MaxKeySize value " + byteToHex(result.maxKeySize) + ": expected 0x10 or 0x20.");
    }

    EXCEPTION_ASSERT_WITH_LOG(offset == data.size(),
        LibLogicalAccessException, "Internal DUOX GetKeySettings parser error : unexpected trailing data.");

    return result;
}

void DUOXISO7816Commands::changeKey(std::uint8_t keyNo, std::shared_ptr<DESFireKey> newKey)
{
    const auto chip = getDESFireChip();

    EXCEPTION_ASSERT_WITH_LOG(chip != nullptr,
        LibLogicalAccessException, "DUOX ChangeKey requires an initialized DESFire chip.");

    const auto crypto = std::dynamic_pointer_cast<DUOXCrypto>(chip->getCrypto());

    EXCEPTION_ASSERT_WITH_LOG(crypto != nullptr,
        LibLogicalAccessException, "DUOX ChangeKey requires DESFireEV2-compatible crypto.");

    if (crypto->d_auth_method == CryptoMethod::CM_EV2)
    {
        DESFireEV2ISO7816Commands::changeKey(keyNo, std::move(newKey));
        return;
    }

    changeKeyEV2Internal(0x00, keyNo, std::move(newKey), DUOXChangeKeyCommand::ChangeKey);
}

void DUOXISO7816Commands::changeKeyEV2(std::uint8_t keySetNo, std::uint8_t keyNo, std::shared_ptr<DESFireKey> newKey)
{
    const auto chip = getDESFireChip();

    EXCEPTION_ASSERT_WITH_LOG(chip != nullptr,
        LibLogicalAccessException, "DUOX ChangeKeyEV2 requires an initialized DESFire chip.");

    const auto crypto = std::dynamic_pointer_cast<DUOXCrypto>(chip->getCrypto());

    EXCEPTION_ASSERT_WITH_LOG(crypto != nullptr,
        LibLogicalAccessException, "DUOX ChangeKeyEV2 requires DESFireEV2-compatible crypto.");

    if (crypto->d_auth_method == CryptoMethod::CM_EV2)
    {
        DESFireEV2ISO7816Commands::changeKeyEV2(keySetNo, keyNo, std::move(newKey));
        return;
    }

    changeKeyEV2Internal(keySetNo, keyNo, std::move(newKey), DUOXChangeKeyCommand::ChangeKeyEV2);
}

void DUOXISO7816Commands::changeKeyEV2Internal(std::uint8_t keySetNo, std::uint8_t keyNo,
    std::shared_ptr<DESFireKey> newKey, DUOXChangeKeyCommand commandType)
{
    const bool isChangeKey = commandType == DUOXChangeKeyCommand::ChangeKey;

    const char *functionName = isChangeKey ? "DUOX ChangeKey" : "DUOX ChangeKeyEV2";

    const auto chip = getDESFireChip();

    EXCEPTION_ASSERT_WITH_LOG(chip != nullptr,
        LibLogicalAccessException, std::string(functionName) + " requires an initialized DESFire chip.");

    const auto crypto = std::dynamic_pointer_cast<DUOXCrypto>(chip->getCrypto());

    EXCEPTION_ASSERT_WITH_LOG(crypto != nullptr,
        LibLogicalAccessException, std::string(functionName) + " requires DESFireEV2 crypto.");

    EXCEPTION_ASSERT_WITH_LOG(newKey != nullptr,
        LibLogicalAccessException, std::string(functionName) + " requires a valid new key.");

    EXCEPTION_ASSERT_WITH_LOG((keySetNo & 0xF0) == 0,
        LibLogicalAccessException, std::string(functionName) + " KeySetNo must use bits 0-3 only.");

    // Keep a private copy. The host-side key store is modified only after the card has successfully accepted ChangeKey
    const auto key = std::make_shared<DESFireKey>(*newKey);

    // This checks only if key type is AES. It does NOT distinguish AES-128 from AES-256
    EXCEPTION_ASSERT_WITH_LOG(key->getKeyType() == DESFireKeyType::DF_KEY_AES,
        LibLogicalAccessException, std::string(functionName) + " requires an AES key.");

    const ByteVector keyData = key->getData();

    EXCEPTION_ASSERT_WITH_LOG(keyData.size() == 16 || keyData.size() == 32,
        LibLogicalAccessException, std::string(functionName) + " requires a 16-byte or 32-byte AES key.");

    const bool piccLevel = crypto->d_currentAid == PICC_LEVEL_AID;

    const std::uint8_t targetKeyNo = static_cast<std::uint8_t>(keyNo & 0x3F);

    if (piccLevel)
        EXCEPTION_ASSERT_WITH_LOG(keySetNo == 0x00,
            LibLogicalAccessException, std::string(functionName) + " requires KeySetNo = 0 at PICC level.");
    
    // Determined before ChangeKey is transmitted because changing the authenticated key invalidates the authentication
    // For ChangeKeyEV2, KeySetNo != 0 always uses the "different key" encoding
    const bool changingAuthenticatedKey = keySetNo == 0x00 && targetKeyNo == crypto->d_currentKeyNo;

    const bool authenticatedEV2 = crypto->d_auth_method == CryptoMethod::CM_DUOX;

    std::uint8_t commandKeyNo = keyNo;

    const bool piccMasterKey = piccLevel && targetKeyNo == 0x00;

    // bits 6 to 7
    if (piccMasterKey)
        commandKeyNo = static_cast<std::uint8_t>(targetKeyNo | (keyData.size() == 16 ? 0x80 : 0xC0));

    const auto oldKey = crypto->getKey(keySetNo, targetKeyNo);

    const ByteVector oldKeyDiversify = oldKey != nullptr ? getKeyInformations(oldKey, targetKeyNo) : ByteVector();

    const ByteVector newKeyDiversify = getKeyInformations(key, targetKeyNo);

    const ByteVector keyDataPlain = crypto->buildChangeKeyData(keySetNo, commandKeyNo, oldKeyDiversify, key, newKeyDiversify);

    const ByteVector params = isChangeKey ? ByteVector{commandKeyNo} : ByteVector{keySetNo, commandKeyNo};

    const std::uint8_t instruction = isChangeKey ? INS_CHANGE_KEY : INS_CHANGE_KEY_EV2;

    ISO7816Response response;
    if (!authenticatedEV2)
    {
        // Not authenticated / no EV2 secure messaging
        response = transmitDUOX(instruction, params, keyDataPlain, DUOXCommunicationMode::Plain);
    }
    else if (changingAuthenticatedKey)
    {
        // EV2 secure messaging applies but plain response because ChangeKey invalidates the authentication
        ByteVector integrityInput;
        integrityInput.reserve(1 + params.size());
        integrityInput.push_back(instruction);
        integrityInput.insert(integrityInput.end(), params.begin(), params.end());

        const ByteVector securedData = crypto->desfireEncrypt(keyDataPlain, integrityInput);

        // The response must NOT be processed with the EV2 session because
        // changing the authenticated key invalidates that authentication
        ByteVector command;
        command.reserve(params.size() + securedData.size());
        command.insert(command.end(), params.begin(), params.end());
        command.insert(command.end(), securedData.begin(), securedData.end());

        response = DESFireISO7816Commands::transmit(instruction, command);
    }
    else
    {
        // Normal EV2 CommMode.Full where command is protected and response is MAC-protected/decrypted
        response = transmitDUOX(instruction, params, keyDataPlain, DUOXCommunicationMode::Full);
    }

    checkSuccess(response, DUOX_SUCCESS, __func__);

    // The card accepted the operation, therefore update the local crypto key store
    crypto->setKey(crypto->d_currentAid, keySetNo, targetKeyNo, key);

    // ChangeKey of the authenticated key invalidates EV2 authentication
    if (changingAuthenticatedKey)
        crypto->invalidateAuthentication();
}

// -------------------------------------------------------------------------
// -------------------------------------------------------------------------
// -------------------------------------------------------------------------

void DUOXISO7816Commands::isoGeneralAuthenticate(
    std::uint8_t caRootKeyNo, std::uint8_t secondaryCaRootKeyNo, CurveID curve,
    bool mutualAuthentication, bool certificatePresent, std::uint8_t certFileNo,
    std::uint8_t privateKeyNo, const ByteVector &privateKey,
    const ByteVector &certificate)
{
    auto &auth = d_duoxECCAuthentication;

    EXCEPTION_ASSERT_WITH_LOG(!auth.active,
        LibLogicalAccessException, "DUOX ECC authentication is already in progress.");

    EXCEPTION_ASSERT_WITH_LOG(caRootKeyNo <= 0x07,
        LibLogicalAccessException, "DUOX CA Root Key number must fit into P2 bits 0-2.");

    EXCEPTION_ASSERT_WITH_LOG(secondaryCaRootKeyNo <= 0x07,
        LibLogicalAccessException, "DUOX secondary CA Root Key number must fit into P2 bits 4-6.");

    if (mutualAuthentication)
    {
        EXCEPTION_ASSERT_WITH_LOG(privateKeyNo <= 0x07,
            LibLogicalAccessException, "DUOX private key number must fit into bits 0-2.");

        EXCEPTION_ASSERT_WITH_LOG(!privateKey.empty(),
            LibLogicalAccessException, "DUOX mutual authentication requires a reader private key.");

        const std::size_t coordinateSize = getCoordinateSize(curve);

        EXCEPTION_ASSERT_WITH_LOG(privateKey.size() == coordinateSize,
            LibLogicalAccessException, "Invalid DUOX reader private key size.");
    }
    else
    {
        // PrivateKeyNo and Priv.A are not used for reader-unilateral authentication
        EXCEPTION_ASSERT_WITH_LOG(privateKey.empty(),
            LibLogicalAccessException, "Reader private key must be omitted for unilateral authentication.");
    }

    if (certificatePresent)
    {
        EXCEPTION_ASSERT_WITH_LOG(!certificate.empty(),
            LibLogicalAccessException, "Cert.A is required when certificate authentication is enabled.");

        EXCEPTION_ASSERT_WITH_LOG(certificate.size() <= 880,
            LibLogicalAccessException, "DUOX Certificate.A exceeds the maximum supported certificate size.");
    }
    else
    {
        EXCEPTION_ASSERT_WITH_LOG(certificate.empty(),
            LibLogicalAccessException, "Cert.A must be empty when certificate authentication is disabled.");
    }

    /*
     * Part 1 creates the authentication state.
     *
     * Part 2 MUST immediately follow it. No other card command is permitted between the two commands
     */
    try
    {
        isoGeneralAuthenticatePart1(caRootKeyNo, secondaryCaRootKeyNo, curve, mutualAuthentication,
                                    certificatePresent, certFileNo, privateKeyNo);
        isoGeneralAuthenticatePart2(privateKey, certificate);
    }
    catch (...)
    {
        /*
         * Authentication context is invalid after any failure.
         *
         * This also guarantees that ephemeral private material is released after a failed authentication
         */
        auth = DUOXECCAuthenticationState{};
        throw;
    }
}

void DUOXISO7816Commands::isoGeneralAuthenticatePart1(
    std::uint8_t caRootKeyNo, std::uint8_t secondaryCaRootKeyNo, CurveID curve,
    bool mutualAuthentication, bool certificatePresent, std::uint8_t certFileNo,
    std::uint8_t privateKeyNo)
{
    printInfo("=== ISOGeneralAuthenticate Part 1 DEBUG ===");

    if (mutualAuthentication)
        EXCEPTION_ASSERT_WITH_LOG(privateKeyNo <= 0x07,
            LibLogicalAccessException, "DUOX private key number must fit into bits 0-2.");

    const std::uint8_t authMethod = mutualAuthentication
                                        ? (certificatePresent ? 0x80 : 0xA0)
                                        : (certificatePresent ? 0x40 : 0x60);

    const ByteVector optsA = buildOptsA(mutualAuthentication, certificatePresent, certFileNo, privateKeyNo);

    // TODO improve : if mutualAuthentication size is 6, if unilateral size is 4
    if (mutualAuthentication && certificatePresent)
        EXCEPTION_ASSERT_WITH_LOG(optsA.size() == 6,
            LibLogicalAccessException, "Unexpected OptsA size for mutual authentication with certificate.");

    /*
     * Generate :
     *   E.Priv.A
     *   E.Pub.A
     * The private key must never leave the authentication state
     */
    const ECKeyPair ephemeralKeyPair = generateECKeyPair(curve);

    EXCEPTION_ASSERT_WITH_LOG(validateECPoint(curve, ephemeralKeyPair.publicKey),
        LibLogicalAccessException, "Generated DUOX ephemeral public key is invalid.");

    const ByteVector encodedEphemeralA = encodeECPoint(curve, ephemeralKeyPair.publicKey);

    EXCEPTION_ASSERT_WITH_LOG(encodedEphemeralA.size() == 65,
        LibLogicalAccessException, "P-256 E.Pub.A must be exactly 65 bytes.");

    EXCEPTION_ASSERT_WITH_LOG(encodedEphemeralA[0] == 0x04,
        LibLogicalAccessException, "E.Pub.A must use uncompressed point representation.");

    const ByteVector authenticationData = buildAuthenticationData(encodedEphemeralA);

    EXCEPTION_ASSERT_WITH_LOG(authenticationData.size() == 69,
        LibLogicalAccessException, "DUOX AuthenticationData must be 69 bytes.");

    const std::uint8_t p2 = buildDUOXGeneralAuthenticateP2(caRootKeyNo, secondaryCaRootKeyNo, true);

    ByteVector commandData;
    commandData.reserve(optsA.size() + authenticationData.size());

    commandData.insert(commandData.end(), optsA.begin(), optsA.end());
    commandData.insert(commandData.end(), authenticationData.begin(), authenticationData.end());

    // Part 1 normally fits in a short APDU (use the extended path if necessary)
    EXCEPTION_ASSERT_WITH_LOG(commandData.size() <= 0xFFFF,
        LibLogicalAccessException, "DUOX ISOGeneralAuthenticate Part 1 command is too large.");

    printInfo("Part 1 : mutual=" + std::to_string(mutualAuthentication) +
              ", certificate=" + std::to_string(certificatePresent) +
              ", CA root=" + std::to_string(caRootKeyNo) +
              ", secondary CA root=" + std::to_string(secondaryCaRootKeyNo) +
              ", curve=" + std::to_string(static_cast<unsigned int>(curve)) +
              ", certFileNo=" + std::to_string(certFileNo) +
              ", privateKeyNo=" + std::to_string(privateKeyNo));

    debugBytes("Part 1 : OptsA", optsA);

    printInfo("Part 1 : sending General Authenticate, Lc=" +
              std::to_string(commandData.size()) + ", P2=0x" + hexDump(ByteVector{p2}));

    const ISO7816Response response = sendDUOXGeneralAuthenticate(p2, commandData, 0x0000); // TODO change Le later

    const std::uint8_t sw1 = response.getSW1();
    const std::uint8_t sw2 = response.getSW2();
    const bool success = sw1 == 0x90 && sw2 == 0x00;
    const bool versionFallback = sw1 == 0x9F && sw2 == 0x00;

    EXCEPTION_ASSERT_WITH_LOG(success || versionFallback,
        LibLogicalAccessException, "DUOX ISOGeneralAuthenticate Part 1 failed.");

    printInfo("Part 1 : response SW=0x" + hexDump(ByteVector{sw1, sw2}) +
              ", data=" + std::to_string(response.getData().size()) + " bytes.");

    const ECPoint ephemeralPublicB = parseAuthenticationResponse(response.getData(), curve);

    EXCEPTION_ASSERT_WITH_LOG(validateECPoint(curve, ephemeralPublicB),
        LibLogicalAccessException, "PICC returned an invalid E.Pub.B point.");

     // ECDH : ShS = ECDH(E.Priv.A, E.Pub.B)
    const ByteVector sharedSecret = deriveECDHSharedSecret(curve, ephemeralKeyPair.privateKey, ephemeralPublicB);

    EXCEPTION_ASSERT_WITH_LOG(!sharedSecret.empty(),
        LibLogicalAccessException, "DUOX ECDH produced an empty shared secret.");

    // Derive K_SESAuthENC K_SESAuthMAC
    const SessionKeys sessionKeys = deriveSessionKeys(ephemeralKeyPair.publicKey, ephemeralPublicB, sharedSecret);

    EXCEPTION_ASSERT_WITH_LOG(sessionKeys.encKey.size() == 16,
        LibLogicalAccessException, "DUOX session ENC key must be 16 bytes.");

    EXCEPTION_ASSERT_WITH_LOG(sessionKeys.macKey.size() == 16,
        LibLogicalAccessException, "DUOX session MAC key must be 16 bytes.");

    DUOXECCAuthenticationState newAuth;
    newAuth.active               = true;
    newAuth.curve                = curve;
    newAuth.mutualAuthentication = mutualAuthentication;
    newAuth.certificatePresent   = certificatePresent;
    newAuth.caRootKeyNo          = caRootKeyNo;
    newAuth.secondaryCaRootKeyNo = secondaryCaRootKeyNo;
    newAuth.certFileNo   = certFileNo;
    newAuth.privateKeyNo = privateKeyNo;
    newAuth.optsA = optsA;
    newAuth.ephemeralPrivateKey = ephemeralKeyPair.privateKey;
    newAuth.ephemeralPublicA    = ephemeralKeyPair.publicKey;
    newAuth.ephemeralPublicB    = ephemeralPublicB;
    newAuth.sessionKeys = sessionKeys;

    d_duoxECCAuthentication = std::move(newAuth);

    printInfo("Part 1 completed : certificatePresent=" + std::to_string(d_duoxECCAuthentication.certificatePresent));
    printInfo("=== ISOGeneralAuthenticate Part 1 DEBUG COMPLETE ===");
}

//TODO Keeping this here temporarily for readability. Will be refactored and correctly integrated into the project later.
namespace 
{
ByteVector padDUOXAES(const ByteVector &data)
{
    constexpr std::size_t blockSize = 16;

    ByteVector padded = data;

    const std::size_t padding = blockSize - (padded.size() % blockSize);

    padded.push_back(0x80);
    padded.insert(padded.end(), padding - 1, 0x00);

    return padded;
}

ByteVector unpadDUOXAES(const ByteVector &data)
{
    EXCEPTION_ASSERT_WITH_LOG(!data.empty() && data.size() % 16 == 0,
        LibLogicalAccessException, "Invalid DUOX AES padded data.");

    std::size_t pos = data.size();

    while (pos > 0 && data[pos - 1] == 0x00)
        --pos;

    EXCEPTION_ASSERT_WITH_LOG(pos > 0 && data[pos - 1] == 0x80,
        LibLogicalAccessException, "Invalid DUOX AES padding.");

    return ByteVector(data.begin(), data.begin() + pos - 1);
}

}
void DUOXISO7816Commands::isoGeneralAuthenticatePart2(const ByteVector &privateKey, const ByteVector &certificate)
{
    printInfo("Executing ISOGeneralAuthenticate Part 2.");

    auto &auth = d_duoxECCAuthentication;

    EXCEPTION_ASSERT_WITH_LOG(auth.active, LibLogicalAccessException,
        "DUOX ISOGeneralAuthenticatePart2 called without an active ISOGeneralAuthenticate Part 1.");

    const bool certAExpected = auth.certificatePresent;
    const bool certAProvided = !certificate.empty();

    EXCEPTION_ASSERT_WITH_LOG(certAExpected == certAProvided,
        LibLogicalAccessException, (certAExpected
             ? "DUOX Part 2 requires Cert.A, but no certificate was provided."
             : "DUOX Part 2 must omit Cert.A, but a certificate was provided."));

    // Part 1 has already established the curve
    const CurveID curve              = auth.curve;
    const std::size_t coordinateSize = getCoordinateSize(auth.curve);

    EXCEPTION_ASSERT_WITH_LOG(privateKey.size() == coordinateSize,
        LibLogicalAccessException, "DUOX static private key does not match the authentication curve.");

    // In the no-Cert.A variants the caller must not provide a certificate
    if (certAExpected)
    {
        EXCEPTION_ASSERT_WITH_LOG(certificate.size() <= 880,
            LibLogicalAccessException, "DUOX Certificate.A exceeds the maximum supported certificate chain size.");

        EXCEPTION_ASSERT_WITH_LOG(certificate.size() >= 4,
            LibLogicalAccessException, "DUOX Cert.A is too short to be a DER certificate.");

        EXCEPTION_ASSERT_WITH_LOG(certificate[0] == 0x30,
            LibLogicalAccessException, "DUOX Cert.A does not start with a DER SEQUENCE.");
    }
    else
        EXCEPTION_ASSERT_WITH_LOG(certificate.empty(),
            LibLogicalAccessException, "Certificate.A must be omitted for this authentication variant.");

    printInfo("Part 2 : certificate=" + std::string(certAExpected ? "present" : "omitted") +
        ", Cert.A size=" + std::to_string(certificate.size()) +
        ", private key size=" + std::to_string(privateKey.size()));

    const ByteVector encodedPublicA = encodeECPoint(curve, auth.ephemeralPublicA);
    const ByteVector encodedPublicB = encodeECPoint(curve, auth.ephemeralPublicB);

    printInfo("E.Pub.A = " + hexDump(encodedPublicA));
    printInfo("E.Pub.B = " + hexDump(encodedPublicB));
    printInfo("E.Pub.A.x = " + hexDump(ByteVector(encodedPublicA.begin() + 1, encodedPublicA.begin() + 33)));
    printInfo("E.Pub.B.x = " + hexDump(ByteVector(encodedPublicB.begin() + 1, encodedPublicB.begin() + 33)));

    ByteVector signedMessage;
    signedMessage.reserve(2 + auth.optsA.size() + encodedPublicA.size() + encodedPublicB.size());

    signedMessage.push_back(0xE0);
    signedMessage.push_back(0xE0);
    signedMessage.insert(signedMessage.end(), auth.optsA.begin(), auth.optsA.end());
    signedMessage.insert(signedMessage.end(), encodedPublicA.begin(), encodedPublicA.end());
    signedMessage.insert(signedMessage.end(), encodedPublicB.begin(), encodedPublicB.end());

    // Generate ECDSA signature using the static reader private key.
    // performDUOXECDSASign() performs SHA-256 internally
    const ECDSASignature signature = signECDSA(curve, privateKey, signedMessage);

    EXCEPTION_ASSERT_WITH_LOG(signature.r.size() == 32 && signature.s.size() == 32,
        LibLogicalAccessException, "DUOX ECDSA signature has an invalid size.");

    // The signature in Msg.A.pl is DER encoded
    const ByteVector encodedSignature = encodeECDSASignatureDER(signature);

    EXCEPTION_ASSERT_WITH_LOG(!encodedSignature.empty(),
        LibLogicalAccessException, "Unable to encode DUOX ECDSA signature.");

    // TEST ONLY (verify the signature using the public key from the exact Cert.A that we are about to send to the card
    //verifySignatureWithCertificate(certificate, signedMessage, signature);

    ByteVector plaintext;
    plaintext.reserve(2 + (auth.certificatePresent ? certificate.size() : 0) + encodedSignature.size());

    plaintext.push_back(0xE0);
    plaintext.push_back(0xE0);
    if (auth.certificatePresent)
        plaintext.insert(plaintext.end(), certificate.begin(), certificate.end());
    plaintext.insert(plaintext.end(), encodedSignature.begin(), encodedSignature.end());

    const std::size_t expectedPlaintextSize = 2 + (certAExpected ? certificate.size() : 0) + encodedSignature.size();

    if (plaintext.size() != expectedPlaintextSize)
    {
        throw std::runtime_error("Msg.A.pl size mismatch: expected " +
            std::to_string(expectedPlaintextSize) + ", got " + std::to_string(plaintext.size()) + ".");
    }
    if (plaintext[0] != 0xE0 || plaintext[1] != 0xE0)
        throw std::runtime_error("Msg.A.pl does not start with E0 E0.");
    const std::size_t signatureOffset = 2 + (certAExpected ? certificate.size() : 0);

    if (!std::equal(certificate.begin(), certificate.end(), plaintext.begin() + 2))
        throw std::runtime_error("Cert.A embedded in Msg.A.pl does not match the supplied certificate.");

    if (!std::equal(encodedSignature.begin(), encodedSignature.end(), plaintext.begin() + signatureOffset))
        throw std::runtime_error("Sig.A embedded in Msg.A.pl does not match generated Sig.A.");

    if (!certAExpected && plaintext.size() != 2 + encodedSignature.size())
        throw std::runtime_error("Msg.A.pl contains unexpected Cert.A bytes.");

    printInfo("Part 2: Msg.A.pl=" + std::to_string(plaintext.size()) +
              " bytes, Cert.A=" + std::to_string(certAExpected ? certificate.size() : 0) +
              " bytes, Sig.A=" + std::to_string(encodedSignature.size()) + " bytes.");

    const ByteVector paddedPlaintext = padDUOXAES(plaintext);
    const ByteVector iv(16, 0x00);

    printInfo("=== DUOX AUTH CRYPTO DEBUG ===");
    printInfo("E.Pub.A = " + hexDump(encodedPublicA));
    printInfo("E.Pub.B = " + hexDump(encodedPublicB));

    const ByteVector xA(encodedPublicA.begin() + 1, encodedPublicA.begin() + 33);
    const ByteVector xB(encodedPublicB.begin() + 1, encodedPublicB.begin() + 33);
    const ByteVector xALast8(xA.end() - 8, xA.end());
    const ByteVector xBLast8(xB.end() - 8, xB.end());
    const ByteVector kdfSalt = [&]()
    {
        ByteVector v;
        v.reserve(16);
        v.insert(v.end(), xALast8.begin(), xALast8.end());
        v.insert(v.end(), xBLast8.begin(), xBLast8.end());
        return v;
    }();

    printInfo("E.Pub.A.x[7..0] = " + hexDump(xALast8));
    printInfo("E.Pub.B.x[7..0] = " + hexDump(xBLast8));
    printInfo("KDF salt         = " + hexDump(kdfSalt));
    printInfo("K_SES_AUTH_ENC    = " + hexDump(auth.sessionKeys.encKey));
    printInfo("K_SES_AUTH_MAC    = " + hexDump(auth.sessionKeys.macKey));
    printInfo("Msg.A.pl          = " + hexDump(plaintext));
    printInfo("Msg.A.pl size      = " + std::to_string(plaintext.size()));
    printInfo("Msg.A.padded      = " + hexDump(paddedPlaintext));

    const ByteVector encryptedMessage = AESHelper::AESEncrypt(paddedPlaintext, auth.sessionKeys.encKey, iv);

    printInfo("Msg.A.enc         = " + hexDump(encryptedMessage));
    printInfo("=== DUOX AUTH CRYPTO DEBUG COMPLETE ===");

    EXCEPTION_ASSERT_WITH_LOG(!encryptedMessage.empty() && encryptedMessage.size() % 16 == 0,
        LibLogicalAccessException, "DUOX encrypted authentication message has an invalid length.");

    auto msgATLV = std::make_shared<TLV>(0x86);
    msgATLV->value(encryptedMessage);

    auto authDO = std::make_shared<TLV>(0x7C);
    authDO->value(msgATLV);

    const ByteVector commandData = authDO->compute_der();

    EXCEPTION_ASSERT_WITH_LOG(commandData.size() <= 0xFFFF,
        LibLogicalAccessException, "DUOX Part 2 authentication data is too large.");

    EXCEPTION_ASSERT_WITH_LOG(commandData.size() >= 2,
        LibLogicalAccessException, "Invalid DUOX Part 2 authentication data.");

    printInfo("Part 2: sending General Authenticate, Lc=" + std::to_string(commandData.size()) + ".");

    ////// TEST /////
    const ByteVector decrypted = AESHelper::AESDecrypt(encryptedMessage, auth.sessionKeys.encKey, iv);
    const ByteVector unpadded = unpadDUOXAES(decrypted);
    if (unpadded != plaintext)
    {
        throw std::runtime_error("Local DUOX Msg.A AES round-trip failed.");
    }
    printInfo("Local Msg.A AES encryption/decryption round-trip verified.");
    /////////////////

    const ISO7816Response response = sendDUOXGeneralAuthenticate(0x00, commandData, 0x0000);

    const std::uint8_t sw1 = response.getSW1();
    const std::uint8_t sw2 = response.getSW2();

    // Part 2 only accepts ISO 9000 as successful completion
    EXCEPTION_ASSERT_WITH_LOG(sw1 == 0x90 && sw2 == 0x00,
        LibLogicalAccessException, "DUOX ISOGeneralAuthenticatePart2 failed.");

    const ByteVector responseData = response.getData();

    auto responseTLVs = TLV::parse_tlvs_der(responseData);

    EXCEPTION_ASSERT_WITH_LOG(responseTLVs.size() == 1,
        LibLogicalAccessException, "DUOX ISOGeneralAuthenticatePart2 returned an invalid authentication data structure.");

    EXCEPTION_ASSERT_WITH_LOG(responseTLVs[0]->tag() == 0x7C,
        LibLogicalAccessException, "DUOX ISOGeneralAuthenticatePart2 returned an invalid authentication data object tag.");

    auto responseAuthenticationTLVs = TLV::parse_tlvs_der(responseTLVs[0]->value());

    EXCEPTION_ASSERT_WITH_LOG(responseAuthenticationTLVs.size() == 1, LibLogicalAccessException,
        "DUOX ISOGeneralAuthenticatePart2 returned an invalid authentication data object count.");

    EXCEPTION_ASSERT_WITH_LOG(responseAuthenticationTLVs[0]->tag() == 0x82,
        LibLogicalAccessException, "DUOX ISOGeneralAuthenticatePart2 returned an invalid Msg.B.enc tag.");

    const ByteVector encryptedMsgB = responseAuthenticationTLVs[0]->value();

    EXCEPTION_ASSERT_WITH_LOG(!encryptedMsgB.empty() && encryptedMsgB.size() % 16 == 0,
        LibLogicalAccessException, "DUOX Msg.B.enc has an invalid encrypted length.");

    const ByteVector paddedMsgB =
        AESHelper::AESDecrypt(encryptedMsgB, auth.sessionKeys.encKey, iv);

    const ByteVector msgB = unpadDUOXAES(paddedMsgB);

    EXCEPTION_ASSERT_WITH_LOG(msgB.size() >= 2,
        LibLogicalAccessException, "DUOX decrypted Msg.B is too short.");

    EXCEPTION_ASSERT_WITH_LOG(msgB[0] == 0xE1 && msgB[1] == 0xE1,
        LibLogicalAccessException, "DUOX Msg.B has an invalid protocol marker.");

    EXCEPTION_ASSERT_WITH_LOG(msgB.size() >= 3,
        LibLogicalAccessException, "DUOX Msg.B is missing OptsB.");

    const std::size_t optsBLength = msgB[2];

    EXCEPTION_ASSERT_WITH_LOG(msgB.size() >= 3 + optsBLength,
        LibLogicalAccessException, "DUOX Msg.B contains a truncated OptsB.");

    ByteVector optsB(msgB.begin() + 2, msgB.begin() + 3 + optsBLength);

    const std::size_t msgBPayloadOffset = 3 + optsBLength;

    printInfo("Part 2: received Msg.B.enc=" + std::to_string(encryptedMsgB.size()) +
              " bytes, OptsB=" + std::to_string(optsB.size()) + " bytes.");

    if (auth.mutualAuthentication)
    {
        EXCEPTION_ASSERT_WITH_LOG(
            msgB.size() > msgBPayloadOffset,
            LibLogicalAccessException, "DUOX mutual authentication response is missing Cert.B/Sig.B.");

        printInfo("Part 2: mutual authentication response contains Cert.B/Sig.B payload (" +
                  std::to_string(msgB.size() - msgBPayloadOffset) + " bytes).");
    }
    else
    {
        EXCEPTION_ASSERT_WITH_LOG(
            msgB.size() == msgBPayloadOffset,
            LibLogicalAccessException, "DUOX reader-unilateral authentication returned unexpected additional data.");
    }

    printInfo("ISOGeneralAuthenticate Part 2 completed.");
}

ISO7816Response DUOXISO7816Commands::sendDUOXGeneralAuthenticate(std::uint8_t p2,
                                                                 const ByteVector &data,
                                                                 std::uint16_t le)
{
    auto adapter = getISO7816ReaderCardAdapter();

    EXCEPTION_ASSERT_WITH_LOG(adapter != nullptr,
        LibLogicalAccessException, "ISO7816 reader/card adapter is required.");

    EXCEPTION_ASSERT_WITH_LOG(data.size() <= 0xFFFF,
        LibLogicalAccessException, "DUOX General Authenticate data is too large.");

    printInfo("sendDUOXGeneralAuthenticate() :");
    printInfo(" CLA = 00");
    printInfo(" INS = 87");
    printInfo(" P1  = 00");
    printInfo(" P2  = " + hexDump(ByteVector{p2}));
    printInfo(" Lc  = " + std::to_string(data.size()));
    debugBytes(" Data", data);
    printInfo(" Le  = 00 00");

    if (data.size() <= 0xFF && le <= 0xFF)
    {
        printInfo("Using normal length APDU: Lc=" + std::to_string(data.size()) + ", Le=" + std::to_string(le));

        return adapter->sendAPDUCommand(ISO7816_CLA_ISO_COMPATIBLE, ISO7816_INS_GENERAL_AUTHENTICATE, 0x00, p2,
                                        static_cast<unsigned char>(data.size()), data, static_cast<unsigned char>(le));
    }
    
    printInfo("Using extended-length APDU: Lc=" + std::to_string(data.size()) + ", Le=" + std::to_string(le));
    return adapter->sendExtendedAPDUCommand(ISO7816_CLA_ISO_COMPATIBLE, ISO7816_INS_GENERAL_AUTHENTICATE, 0x00, p2,
                                            static_cast<unsigned short>(data.size()), data, le);
}

// TODO should be 5 instead of 7
std::uint8_t DUOXISO7816Commands::buildDUOXGeneralAuthenticateP2(std::uint8_t caRootKeyNo,
                                                    std::uint8_t secondaryCaRootKeyNo,
                                                    bool multipleApplicationSelection)
{
    EXCEPTION_ASSERT_WITH_LOG(caRootKeyNo <= 0x07,
        LibLogicalAccessException, "Invalid DUOX CA root key number.");

    if (multipleApplicationSelection)
        EXCEPTION_ASSERT_WITH_LOG(secondaryCaRootKeyNo <= 0x07,
            LibLogicalAccessException, "Invalid DUOX secondary CA root key number.");

    return static_cast<std::uint8_t>(caRootKeyNo | (multipleApplicationSelection ?
        static_cast<std::uint8_t>(secondaryCaRootKeyNo << 4) : 0x00));
}

ByteVector DUOXISO7816Commands::buildOptsA(bool mutualAuthentication, bool certificateAIncluded,
                                               std::uint8_t certificateFileNo, std::uint8_t privateKeyNo)
{
    std::uint8_t authMethod;

    if (mutualAuthentication)
    {
        authMethod = certificateAIncluded ? 0x80 : 0xA0;
    }
    else
    {
        authMethod = certificateAIncluded ? 0x40 : 0x60;
    }

    ByteVector value;
    value.push_back(authMethod);
    value.push_back(0x00); // ProtocolVersion
    if (mutualAuthentication)
    {
        value.push_back(certificateFileNo);
        value.push_back(privateKeyNo);
    }

    ByteVector result;
    result.push_back(0x80);
    result.push_back(static_cast<std::uint8_t>(value.size())); // Length is only 2 or 4
    result.insert(result.end(), value.begin(), value.end());

    return result;
}

ByteVector DUOXISO7816Commands::buildAuthenticationData(const ByteVector &ephemeralPublicKey)
{
    EXCEPTION_ASSERT_WITH_LOG(ephemeralPublicKey.size() == 65,
        LibLogicalAccessException, "DUOX ephemeral public key must contain 65 bytes.");

    ByteVector result;
    // TODO result.reserve 69 bytes

    // Authentication Data Objects header
    result.push_back(0x7C);
    result.push_back(0x43);

    // E.Pub.A
    result.push_back(0x85);
    result.push_back(0x41);

    result.insert(result.end(), ephemeralPublicKey.begin(), ephemeralPublicKey.end());

    return result;
}

// TODO Outdated. Refactor after Part2 is completely refactored.
ECPoint DUOXISO7816Commands::parseAuthenticationResponse(const ByteVector &response, CurveID curve)
{
    const auto tlvs = TLV::parse_tlvs_der(response, true);

    EXCEPTION_ASSERT_WITH_LOG(tlvs.size() == 1,
        LibLogicalAccessException, "Expected exactly one DUOX authentication TLV.");

    const auto &authDO = tlvs.front();

    EXCEPTION_ASSERT_WITH_LOG(authDO->tag() == 0x7C,
        LibLogicalAccessException, "Expected DUOX authentication data object 7C.");

    EXCEPTION_ASSERT_WITH_LOG(authDO->getSizeTag() == 0x43,
        LibLogicalAccessException, "Expected DUOX authentication data object 7C length to be 0x43.");

    EXCEPTION_ASSERT_WITH_LOG(authDO->value().size() == 0x43,
        LibLogicalAccessException, "DUOX authentication data object 7C must contain 0x43 bytes.");

    const auto nodes = TLV::parse_tlvs_der(authDO->value(), true);

    EXCEPTION_ASSERT_WITH_LOG(nodes.size() == 1,
        LibLogicalAccessException, "Expected exactly one DUOX authentication node.");

    const auto &ePubB = nodes.front();

    EXCEPTION_ASSERT_WITH_LOG(ePubB->tag() == 0x85,
        LibLogicalAccessException, "Expected DUOX E.Pub.B tag 85.");

    EXCEPTION_ASSERT_WITH_LOG(ePubB->getSizeTag() == 0x41,
        LibLogicalAccessException, "Expected DUOX E.Pub.B length to be 0x41.");

    const ByteVector encodedPoint = ePubB->value();

    EXCEPTION_ASSERT_WITH_LOG(encodedPoint.size() == 0x41,
        LibLogicalAccessException, "DUOX E.Pub.B must contain 65 bytes.");

    return decodeECPoint(curve, encodedPoint);
}

ByteVector DUOXISO7816Commands::isoSelectFile(std::uint8_t p1, std::uint8_t p2, const ByteVector &data, std::uint32_t le)
{
    constexpr std::uint8_t DUOX_ISO_CLA             = 0x00;
    constexpr std::uint8_t DUOX_ISO_INS_SELECT_FILE = 0xA4;
    constexpr std::uint8_t DUOX_ISO_P2_RETURN_FCI   = 0x00;
    constexpr std::uint8_t DUOX_ISO_P2_NO_FCI       = 0x0C;

    constexpr std::size_t DUOX_ISO_SELECT_FILE_MAX_DATA_LENGTH = 16;
    constexpr std::uint32_t DUOX_ISO_SELECT_FILE_MAX_LE        = 65536;

    EXCEPTION_ASSERT_WITH_LOG(p1 <= 0x04,
        LibLogicalAccessException, "Invalid P1. Expected a value in the range 0x00...0x04.");

    EXCEPTION_ASSERT_WITH_LOG(p2 == DUOX_ISO_P2_RETURN_FCI || p2 == DUOX_ISO_P2_NO_FCI,
        LibLogicalAccessException, "Invalid P2. Expected 0x00 or 0x0C.");

    EXCEPTION_ASSERT_WITH_LOG(data.size() <= DUOX_ISO_SELECT_FILE_MAX_DATA_LENGTH,
        LibLogicalAccessException, "Selection data cannot exceed 16 bytes.");

    EXCEPTION_ASSERT_WITH_LOG(le <= DUOX_ISO_SELECT_FILE_MAX_LE,
        LibLogicalAccessException, "Le cannot exceed 65536 bytes.");

    switch (p1)
    {
    case 0x00:
    case 0x01:
    case 0x02:
        /*
         * For P1 == 00/01/02 :
         *   - Normal file selection requires a two-byte ISO FID
         *   - MF selection may use an empty data field
         * Distinction between those cases belongs to the PICC/card
         */
        EXCEPTION_ASSERT_WITH_LOG(data.empty() || data.size() == 2,
            LibLogicalAccessException, "P1 = 0x00 or 0x01 or 0x02 requires either an empty selection reference or exactly "
            "a two-byte file identifier.");
        break;

    case 0x03:
        EXCEPTION_ASSERT_WITH_LOG(data.empty(), LibLogicalAccessException, "P1 = 0x03 requires an empty data field.");
        break;

    case 0x04:
        EXCEPTION_ASSERT_WITH_LOG(!data.empty(), LibLogicalAccessException, "P1 = 0x04 requires a DF name.");
        EXCEPTION_ASSERT_WITH_LOG(data.size() <= 16, LibLogicalAccessException, "DF name cannot exceed 16 bytes.");
        break;

    default: // Keep the default for defensive programming if the function is modified later
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Unreachable invalid P1.");
    }

    auto adapter = getISO7816ReaderCardAdapter();

    EXCEPTION_ASSERT_WITH_LOG(adapter != nullptr,
        LibLogicalAccessException, "ISO7816 reader/card adapter is not initialized.");

    // Short APDU
    if (le <= 256)
    {
        const auto encodedLe = static_cast<std::uint8_t>(le == 256 ? 0x00 : le);
        const auto response = adapter->sendAPDUCommand(
            DUOX_ISO_CLA, DUOX_ISO_INS_SELECT_FILE, p1, p2, static_cast<unsigned char>(data.size()), data, encodedLe);

        /*
         * VERY IMPORTANT ! 
         * 0x9000 = successful execution
         * 0x6283 = application selected with limited functionality :
         * It's a warning rather than a true transport failure and the application is indeed selected
         */
        // TODO rework : SUCCESS (0x9000) shouldn't be specified because it's the normal SW output
        EXCEPTION_ASSERT_WITH_LOG(isSelectFileSuccess(response, SUCCESS) ||
                isSelectFileSuccess(response, DUOX_SUCCESS) ||
                isSelectFileSuccess(response, DUOX_FILE_LIMITED_FUNCTIONALITY),
            LibLogicalAccessException, "DUOX ISOSelectFile failed with status word " + byteToHex(response.getSW1()) + " " +
                                      byteToHex(response.getSW2()) + ".");

        validateFileResponseData(response.getData());

        return response.getData();
    }

    const auto extendedLe = static_cast<std::uint16_t>(le == 65536 ? 0x0000 : le);
    const auto response = adapter->sendExtendedAPDUCommand(
        DUOX_ISO_CLA, DUOX_ISO_INS_SELECT_FILE, p1, p2, static_cast<unsigned short>(data.size()), data, extendedLe);

    // TODO rework : SUCCESS (0x9000) shouldn't be specified because it's normal SW output
    EXCEPTION_ASSERT_WITH_LOG(isSelectFileSuccess(response, SUCCESS) ||
                isSelectFileSuccess(response, DUOX_SUCCESS) ||
                isSelectFileSuccess(response, DUOX_FILE_LIMITED_FUNCTIONALITY),
        LibLogicalAccessException, "DUOX ISOSelectFile failed with status word " + byteToHex(response.getSW1()) + " " +
                                  byteToHex(response.getSW2()) + ".");

    validateFileResponseData(response.getData());

    return response.getData();
}

ByteVector DUOXISO7816Commands::isoSelectFile(std::uint8_t p1, std::uint8_t p2, const ByteVector &data)
{
    return isoSelectFile(p1, p2, data, 0x00);
}

ByteVector DUOXISO7816Commands::isoSelectFile(const ByteVector &dfName, bool returnFCI)
{
    constexpr std::uint8_t ISO_P2_RETURN_FCI = 0x00;
    constexpr std::uint8_t ISO_P2_NO_FCI     = 0x0C;
    return isoSelectFile(0x04, returnFCI ? ISO_P2_RETURN_FCI : ISO_P2_NO_FCI, dfName, 0x00);
}

ByteVector DUOXISO7816Commands::isoSelectApplicationWithECCFCI(const ByteVector &dfName, std::uint32_t le)
{
    constexpr std::uint32_t MIN_LE = 69;

    EXCEPTION_ASSERT_WITH_LOG(le == 0 || le >= MIN_LE,
        LibLogicalAccessException, "Le must be 0 (maximum short response length) "
        "or at least 69 bytes for integrated ECC-based authentication.");

    const auto response = isoSelectFile(0x04, 0x00, dfName, le);

    EXCEPTION_ASSERT_WITH_LOG(isValidEphemeralPublicKeyFCI(response),
        LibLogicalAccessException, "Expected DUOX ephemeral public-key FCI for integrated ECC-based authentication.");

    return response;
}

ByteVector DUOXISO7816Commands::isoSelectApplicationForFastUID(const ByteVector &dfName, std::uint32_t le)
{
    constexpr std::uint32_t MIN_LE = 36;

    EXCEPTION_ASSERT_WITH_LOG(le == 0 || le >= MIN_LE,
        LibLogicalAccessException, "Le must be 0 (maximum short response length) "
        "or at least 36 bytes for fast UID retrieval.");

    const auto response = isoSelectFile(0x04, 0x00, dfName, le);

    EXCEPTION_ASSERT_WITH_LOG(isValidEncryptedUIDFCI(response),
        LibLogicalAccessException, "Expected DUOX encrypted UID FCI for fast UID retrieval.");

    return response;
}

ByteVector DUOXISO7816Commands::parseGeneralAuthenticateFinalResponse(const ByteVector &data)
{
    constexpr std::uint8_t TAG_AUTHENTICATION_DATA_OBJECTS = 0x7C;
    constexpr std::uint8_t TAG_ENCRYPTED_MESSAGE = 0x82;

    EXCEPTION_ASSERT_WITH_LOG(data.size() >= 4,
        LibLogicalAccessException, "ISOGeneralAuthenticateFinal returned an invalid response : "
        "Authentication Data Objects are missing or truncated.");

    std::size_t offset = 0;

    EXCEPTION_ASSERT_WITH_LOG(data[offset++] == TAG_AUTHENTICATION_DATA_OBJECTS,
        LibLogicalAccessException, "ISOGeneralAuthenticateFinal returned an invalid Authentication Data Objects tag.");

    const auto outerLength = parseDERLength(data, offset);

    EXCEPTION_ASSERT_WITH_LOG(outerLength == data.size() - offset,
        LibLogicalAccessException, "ISOGeneralAuthenticateFinal returned an invalid Authentication Data Objects length.");

    const auto outerEnd = offset + outerLength;

    EXCEPTION_ASSERT_WITH_LOG(offset < outerEnd,
        LibLogicalAccessException, "ISOGeneralAuthenticateFinal returned an empty Authentication Data Objects structure.");

    EXCEPTION_ASSERT_WITH_LOG(data[offset++] == TAG_ENCRYPTED_MESSAGE,
        LibLogicalAccessException, "ISOGeneralAuthenticateFinal returned an invalid Msg.B.enc tag.");

    const auto msgBLength = parseDERLength(data, offset);

    EXCEPTION_ASSERT_WITH_LOG(msgBLength == outerEnd - offset,
        LibLogicalAccessException, "ISOGeneralAuthenticateFinal returned an invalid Msg.B.enc length.");

    return ByteVector(data.begin() + offset, data.end());
}

ByteVector DUOXISO7816Commands::isoGeneralAuthenticateFinal(std::uint8_t caRootKeyNo, AuthMethod authMethod,
    const ByteVector &certA, std::uint8_t certFileNo, std::uint8_t privateKeyNo, const ByteVector &ePublicA,
    const ByteVector &msgAEnc, CurveID curve, std::uint32_t le)
{
    constexpr std::uint8_t CLA = 0x00;
    constexpr std::uint8_t INS = 0x87;
    constexpr std::uint8_t P1  = 0x00;

    constexpr std::uint8_t MAX_CA_ROOT_KEY_NO = 0x04;
    constexpr std::uint8_t MAX_PRIVATE_KEY_NO = 0x04;

    // Only 5 CaRootKeys can exist per application
    EXCEPTION_ASSERT_WITH_LOG(caRootKeyNo <= MAX_CA_ROOT_KEY_NO,
        LibLogicalAccessException, "Invalid DUOX CA Root Key number : expected 0x00...0x04.");

    EXCEPTION_ASSERT_WITH_LOG(
        isSupportedAuthMethod(authMethod), LibLogicalAccessException,
        "Invalid DUOX ISOGeneralAuthenticateFinal authentication method.");

    const bool certificateRequired = usesCertificate(authMethod);

    if (certificateRequired)
    {
        EXCEPTION_ASSERT_WITH_LOG(!certA.empty(),
            LibLogicalAccessException, "Cert.A required for the selected authentication method.");
    }
    else
    {
        EXCEPTION_ASSERT_WITH_LOG(certA.empty(),
            LibLogicalAccessException,
            "Cert.A must be empty when the selected authentication method doesn't use a certificate.");

        EXCEPTION_ASSERT_WITH_LOG(certFileNo == 0x00,
            LibLogicalAccessException, "CertFileNo must be zero when Cert.A isn't used.");
    }

    EXCEPTION_ASSERT_WITH_LOG(privateKeyNo <= MAX_PRIVATE_KEY_NO,
        LibLogicalAccessException, "Invalid DUOX private key number : expected 0x00...0x04.");

    EXCEPTION_ASSERT_WITH_LOG(ePublicA.size() == 65,
        LibLogicalAccessException, "E.Pub.A must contain exactly 65 bytes.");

    EXCEPTION_ASSERT_WITH_LOG(ePublicA[0] == 0x04,
        LibLogicalAccessException, "E.Pub.A must use uncompressed point representation.");

    EXCEPTION_ASSERT_WITH_LOG(!msgAEnc.empty(),
        LibLogicalAccessException, "Msg.A.enc cannot be empty.");

    EXCEPTION_ASSERT_WITH_LOG(le <= 65536,
        LibLogicalAccessException, "Le cannot exceed 65536.");

    auto adapter = getISO7816ReaderCardAdapter();

    EXCEPTION_ASSERT_WITH_LOG(adapter != nullptr,
        LibLogicalAccessException, "ISO7816 reader/card adapter is not initialized.");

    const std::uint8_t p2 = caRootKeyNo & 0x07;

    // OptsA
    ByteVector optsA;
    if (certificateRequired)
    {
        optsA = {0x80, 0x04, static_cast<std::uint8_t>(authMethod), 0x00, certFileNo, privateKeyNo};
    }
    else
    {
        optsA = {0x80, 0x02, static_cast<std::uint8_t>(authMethod), 0x00};
    }

    // E.Pub.A
    ByteVector ePublicATLV;
    ePublicATLV.reserve(67);

    ePublicATLV.push_back(0x85);
    ePublicATLV.push_back(0x41);
    ePublicATLV.insert(ePublicATLV.end(), ePublicA.begin(), ePublicA.end());

    // Msg.A.enc
    ByteVector msgAEncTLV;
    msgAEncTLV.reserve(2 + msgAEnc.size());

    msgAEncTLV.push_back(0x86);
    msgAEncTLV.push_back(static_cast<std::uint8_t>(msgAEnc.size()));
    msgAEncTLV.insert(msgAEncTLV.end(), msgAEnc.begin(), msgAEnc.end());

    // Authentication Data Objects (TODO : support extended BER-style length encoding)
    ByteVector authenticationDataObjects;
    authenticationDataObjects.reserve(ePublicATLV.size() + msgAEncTLV.size());
    authenticationDataObjects.insert(authenticationDataObjects.end(), ePublicATLV.begin(), ePublicATLV.end());
    authenticationDataObjects.insert(authenticationDataObjects.end(), msgAEncTLV.begin(), msgAEncTLV.end());

    ByteVector authDO;
    authDO.reserve(2 + authenticationDataObjects.size());

    authDO.push_back(0x7C);
    authDO.push_back(static_cast<std::uint8_t>(authenticationDataObjects.size()));
    authDO.insert(authDO.end(), authenticationDataObjects.begin(), authenticationDataObjects.end());

    // Complete command data
    ByteVector commandData;
    commandData.reserve(optsA.size() + authDO.size());

    commandData.insert(commandData.end(), optsA.begin(), optsA.end());
    commandData.insert(commandData.end(), authDO.begin(), authDO.end());

    EXCEPTION_ASSERT_WITH_LOG(commandData.size() <= 0xFFFF,
        LibLogicalAccessException, "DUOX ISOGeneralAuthenticateFinal data exceeds the maximum extended APDU length.");

    ISO7816Response response;

    // Short APDU
    if (le <= 256)
    {
        const auto encodedLe = static_cast<std::uint8_t>(le == 256 ? 0x00 : le);

        EXCEPTION_ASSERT_WITH_LOG(commandData.size() <= 255,
            LibLogicalAccessException, "DUOX ISOGeneralAuthenticateFinal command data is too large for a short APDU.");

        response = adapter->sendAPDUCommand(
            CLA, INS, P1, p2, static_cast<unsigned char>(commandData.size()), commandData, encodedLe);
    }
    else
    {
        const auto encodedLe = static_cast<std::uint16_t>(le == 65536 ? 0x0000 : le);

        response = adapter->sendExtendedAPDUCommand(
            CLA, INS, P1, p2, static_cast<unsigned short>(commandData.size()), commandData, encodedLe);
    }

    EXCEPTION_ASSERT_WITH_LOG(
        response.getSW1() == 0x90 && response.getSW2() == 0x00, LibLogicalAccessException,
        "DUOX ISOGeneralAuthenticateFinal failed with status word " +
            byteToHex(response.getSW1()) + " " + byteToHex(response.getSW2()) + ".");

    checkSuccess(response, DUOX_SUCCESS, __func__);

    return parseGeneralAuthenticateFinalResponse(response.getData());
}

ByteVector DUOXISO7816Commands::isoInternalAuthenticate(unsigned char keyno,
    const ByteVector &rndA, const ByteVector &optsA)
{
    constexpr unsigned char ISO_INTERNAL_AUTHENTICATE_P1  = 0x00;

    constexpr unsigned char ISO_INTERNAL_AUTHENTICATE_TAG_OPTS_A    = 0x80;
    constexpr unsigned char ISO_INTERNAL_AUTHENTICATE_TAG_AUTH_DATA = 0x7C;
    constexpr unsigned char ISO_INTERNAL_AUTHENTICATE_TAG_RNDA      = 0x81;
    constexpr unsigned char ISO_INTERNAL_AUTHENTICATE_TAG_RNDB      = 0x81;
    constexpr unsigned char ISO_INTERNAL_AUTHENTICATE_TAG_SIG_B     = 0x82;

    constexpr size_t ISO_INTERNAL_AUTHENTICATE_RAND_LENGTH      = 16;
    constexpr size_t ISO_INTERNAL_AUTHENTICATE_SIGNATURE_LENGTH = 64;
    // 0x7C / 0x54 + RndB TLV + Sig.B TLV = 86 bytes
    constexpr size_t ISO_INTERNAL_AUTHENTICATE_RESPONSE_LENGTH =
        2 + (2 + ISO_INTERNAL_AUTHENTICATE_RAND_LENGTH) +
        (2 + ISO_INTERNAL_AUTHENTICATE_SIGNATURE_LENGTH);

    // Authentication Data Object
    constexpr size_t ISO_INTERNAL_AUTHENTICATE_AUTH_DATA_LENGTH = 2 + 2 + ISO_INTERNAL_AUTHENTICATE_RAND_LENGTH;
    constexpr size_t ISO_INTERNAL_AUTHENTICATE_MAX_OPTS_A_LENGTH = 0xE9;

    EXCEPTION_ASSERT_WITH_LOG(rndA.size() == ISO_INTERNAL_AUTHENTICATE_RAND_LENGTH,
        LibLogicalAccessException, "ISOInternalAuthenticate : RndA must contain exactly 16 bytes.");

    EXCEPTION_ASSERT_WITH_LOG(optsA.size() <= ISO_INTERNAL_AUTHENTICATE_MAX_OPTS_A_LENGTH,
        LibLogicalAccessException, "ISOInternalAuthenticate : OptsA exceeds the maximum supported length (0xE9 bytes).");

    EXCEPTION_ASSERT_WITH_LOG(keyno <= 0x07,
        LibLogicalAccessException, "ISOInternalAuthenticate : key number must fit in P2 bits 2...0.");

    //Build 7C 12 81 10 RndA
    auto rndA_tlv = std::make_shared<TLV>(ISO_INTERNAL_AUTHENTICATE_TAG_RNDA);
    rndA_tlv->value(rndA);

    auto auth_data_tlv = std::make_shared<TLV>(ISO_INTERNAL_AUTHENTICATE_TAG_AUTH_DATA);
    auth_data_tlv->value(rndA_tlv);

    const ByteVector auth_data_encoded = auth_data_tlv->compute_der();//auth_data_tlv->compute();

    EXCEPTION_ASSERT_WITH_LOG(auth_data_encoded.size() == ISO_INTERNAL_AUTHENTICATE_AUTH_DATA_LENGTH,
        LibLogicalAccessException, "ISOInternalAuthenticate: failed to construct the authentication data object.");

    ByteVector data;

    if (!optsA.empty())
    {
        auto optsA_tlv = std::make_shared<TLV>(ISO_INTERNAL_AUTHENTICATE_TAG_OPTS_A);
        optsA_tlv->value(optsA);

        const ByteVector optsA_encoded = optsA_tlv->compute_der(); // optsA_tlv->compute();

        EXCEPTION_ASSERT_WITH_LOG(optsA_encoded.size() == optsA.size() + (optsA.size() < 0x80 ? 2 : 3),
            LibLogicalAccessException, "ISOInternalAuthenticate : invalid OptsA DER encoding.");
        //EXCEPTION_ASSERT_WITH_LOG(optsA_encoded.size() == optsA.size() + 2,
        //    LibLogicalAccessException, "ISOInternalAuthenticate : invalid OptsA TLV encoding.");
        EXCEPTION_ASSERT_WITH_LOG(optsA_encoded.size() <= 235,
            LibLogicalAccessException, "ISOInternalAuthenticate : encoded OptsA exceeds 235 bytes.");

        data.insert(data.end(), optsA_encoded.begin(), optsA_encoded.end());
    }

    data.insert(data.end(), auth_data_encoded.begin(), auth_data_encoded.end());

    EXCEPTION_ASSERT_WITH_LOG(data.size() >= ISO_INTERNAL_AUTHENTICATE_AUTH_DATA_LENGTH,
        LibLogicalAccessException, "ISOInternalAuthenticate : malformed command data.");

    EXCEPTION_ASSERT_WITH_LOG(data.size() <= 0xFF,
        LibLogicalAccessException, "ISOInternalAuthenticate : command data exceeds short APDU capacity.");

    constexpr unsigned char expected_response_length = static_cast<unsigned char>(ISO_INTERNAL_AUTHENTICATE_RESPONSE_LENGTH);

    const unsigned char p2 = static_cast<unsigned char>(keyno & 0x07);

    const ByteVector response = getISO7816ReaderCardAdapter()->sendAPDUCommand(
                ISO7816_CLA_ISO_COMPATIBLE, ISO7816_INS_INTERNAL_AUTHENTICATE, ISO_INTERNAL_AUTHENTICATE_P1, p2,
                static_cast<unsigned char>(data.size()), data, expected_response_length).getData();

    // Validate the complete response before returning it
    // Note that malformed authentication material can turn card/protocol error into
    // an apparently valid cryptographic verification failure !

    EXCEPTION_ASSERT_WITH_LOG(response.size() == ISO_INTERNAL_AUTHENTICATE_RESPONSE_LENGTH,
        LibLogicalAccessException, "ISOInternalAuthenticate : invalid response length.");

    const auto response_tlvs = TLV::parse_tlvs_der(response, true);//TLV::parse_tlvs(response);

    EXCEPTION_ASSERT_WITH_LOG(response_tlvs.size() == 1,
        LibLogicalAccessException, "ISOInternalAuthenticate : invalid response TLV structure.");

    const auto &auth_response = response_tlvs.at(0);

    EXCEPTION_ASSERT_WITH_LOG(auth_response->tag() == ISO7816_DATA_OBJECT_DYNAMIC_AUTHENTICATION_DATA,
        LibLogicalAccessException, "ISOInternalAuthenticate : missing authentication-data response TLV.");

    const auto response_nodes = TLV::parse_tlvs_der(auth_response->value(), true);//TLV::parse_tlvs(auth_response->value());

    EXCEPTION_ASSERT_WITH_LOG(response_nodes.size() == 2,
        LibLogicalAccessException, "ISOInternalAuthenticate : expected exactly RndB and Sig.B.");

    const auto &rndB_tlv = response_nodes.at(0);
    const auto &sigB_tlv = response_nodes.at(1);

    EXCEPTION_ASSERT_WITH_LOG(rndB_tlv->tag() == ISO_INTERNAL_AUTHENTICATE_TAG_RNDB,
        LibLogicalAccessException, "ISOInternalAuthenticate : unexpected RndB TLV tag.");

    EXCEPTION_ASSERT_WITH_LOG(rndB_tlv->value().size() == ISO_INTERNAL_AUTHENTICATE_RAND_LENGTH,
        LibLogicalAccessException, "ISOInternalAuthenticate : RndB must contain exactly 16 bytes.");

    EXCEPTION_ASSERT_WITH_LOG(sigB_tlv->tag() == ISO_INTERNAL_AUTHENTICATE_TAG_SIG_B,
        LibLogicalAccessException, "ISOInternalAuthenticate : unexpected Sig.B TLV tag.");

    EXCEPTION_ASSERT_WITH_LOG(sigB_tlv->value().size() == ISO_INTERNAL_AUTHENTICATE_SIGNATURE_LENGTH,
        LibLogicalAccessException, "ISOInternalAuthenticate : Sig.B must contain exactly 64 bytes.");

    return response;
}

DESFireCommands::DESFireCardVersion DUOXISO7816Commands::getVersion()
{
    return getVersionImpl(DESFireGetVersionOption::FabID, false);
}

std::uint8_t DUOXISO7816Commands::buildEV2KeyNo(std::uint8_t targetedKeyNo, bool secondaryApplication)
{
    EXCEPTION_ASSERT_WITH_LOG(targetedKeyNo <= KEYNO_MASK,
        LibLogicalAccessException, "DUOX EV2 AuthenticateEV2First targeted key number must fit in the six-bit KeyNo field.");

    const std::uint8_t keyNo = static_cast<std::uint8_t>(
        (secondaryApplication ? KEYNO_SECONDARY_APPLICATION : 0x00) | (targetedKeyNo & KEYNO_MASK));

    EXCEPTION_ASSERT_WITH_LOG((keyNo & KEYNO_RESERVED) == 0,
        LibLogicalAccessException, "Internal DUOX EV2 error : constructed KeyNo contains RFU bit 6.");

    return keyNo;
}

ByteVector DUOXISO7816Commands::validateEV2Part2Response(
    const ByteVector &response, const ByteVector &rndA, const ByteVector &expectedPCDCap2)
{
    EXCEPTION_ASSERT_WITH_LOG(response.size() == EV2_PART2_RESPONSE_SIZE,
        LibLogicalAccessException, "DUOX AuthenticateEV2First Part 2 decrypted response must contain exactly 32 bytes.");

    EXCEPTION_ASSERT_WITH_LOG(rndA.size() == EV2_RANDOM_SIZE,
        LibLogicalAccessException, "DUOX AuthenticateEV2First validation requires a 16-byte RndA.");

    EXCEPTION_ASSERT_WITH_LOG(expectedPCDCap2.size() <= EV2_PCDCAP2_SIZE,
        LibLogicalAccessException, "DUOX PCDcap2 must contain at most six bytes.");

    const ByteVector expectedRndAPrime = DUOXCrypto::rotateAuthenticationChallenge(rndA);

    const ByteVector receivedRndAPrime(response.begin() + EV2_TI_SIZE,
                                       response.begin() + EV2_TI_SIZE + EV2_RANDOM_SIZE);

    EXCEPTION_ASSERT_WITH_LOG(receivedRndAPrime == expectedRndAPrime, LibLogicalAccessException,
        "DUOX AuthenticateEV2First failed: received RndA' does not match the expected rotated RndA.");

    /*
     * PDcap2 is supplied by the PICC.
     * It' is 's not consumed by the command layer but it's part of the authenticated response
     * and therefore deliberately parsed here rather than silently skipping six bytes.
     */
    const ByteVector piccCap2(response.begin() + EV2_TI_SIZE + EV2_RANDOM_SIZE,
                              response.begin() + EV2_TI_SIZE + EV2_RANDOM_SIZE + EV2_PDCAP2_SIZE);

    EXCEPTION_ASSERT_WITH_LOG(piccCap2.size() == EV2_PDCAP2_SIZE,
        LibLogicalAccessException, "DUOX AuthenticateEV2First returned an invalid PDcap2 length.");

    ByteVector expectedAdjustedPCDCap2 = expectedPCDCap2;
    expectedAdjustedPCDCap2.resize(EV2_PCDCAP2_SIZE, static_cast<std::uint8_t>(0x00));
    
    const ByteVector returnedPCDCap2(response.begin() + EV2_TI_SIZE + EV2_RANDOM_SIZE + EV2_PDCAP2_SIZE,
                                     response.end());

    EXCEPTION_ASSERT_WITH_LOG(returnedPCDCap2.size() == EV2_PCDCAP2_SIZE,
        LibLogicalAccessException, "DUOX AuthenticateEV2First returned an invalid PCDcap2 length.");

    EXCEPTION_ASSERT_WITH_LOG(returnedPCDCap2 == expectedAdjustedPCDCap2, LibLogicalAccessException,
        "DUOX AuthenticateEV2First returned PCDcap2 does not match the adjusted capabilities sent by the PCD.");

    return ByteVector(response.begin(), response.begin() + EV2_TI_SIZE);
}

void DUOXISO7816Commands::authenticateEV2First(std::uint8_t keyno, std::shared_ptr<DESFireKey> currentKey)
{
    EXCEPTION_ASSERT_WITH_LOG((keyno & KEYNO_RESERVED) == 0,
        LibLogicalAccessException, "DUOX EV2 AuthenticateEV2First : KeyNo bit 6 is reserved and must be zero.");

    const bool secondaryApplication  = (keyno & KEYNO_SECONDARY_APPLICATION) != 0;
    const std::uint8_t targetedKeyNo = static_cast<std::uint8_t>(keyno & KEYNO_MASK);

    authenticateEV2First(targetedKeyNo, std::move(currentKey), secondaryApplication, ByteVector());
}

void DUOXISO7816Commands::authenticateEV2First(std::uint8_t targetedKeyNo, std::shared_ptr<DESFireKey> key,
                                               bool secondaryApplication, ByteVector pcdCap2)
{
    constexpr std::size_t MAX_PCDCAP2_SIZE = 6;
    constexpr std::uint8_t SW_SUCCESS      = 0x00;

    EXCEPTION_ASSERT_WITH_LOG(targetedKeyNo <= KEYNO_MASK,
        LibLogicalAccessException, "DUOX EV2 AuthenticateEV2First targeted key number must be in the range 0x00..0x3F.");

    // TODO : PCDcap2 might eventually be added as persistent class state later
    EXCEPTION_ASSERT_WITH_LOG(pcdCap2.size() <= MAX_PCDCAP2_SIZE,
        LibLogicalAccessException, "DUOX EV2 AuthenticateEV2First PCDcap2 must contain at most six bytes.");

    const auto chip = getDESFireChip();

    EXCEPTION_ASSERT_WITH_LOG(chip != nullptr,
        LibLogicalAccessException, "DUOX EV2 AuthenticateEV2First requires an initialized DESFire chip.");

    const auto crypto = chip->getCrypto();

    EXCEPTION_ASSERT_WITH_LOG(crypto != nullptr,
        LibLogicalAccessException, "DUOX EV2 AuthenticateEV2First requires an initialized DESFire crypto context.");

    const auto duoxCrypto = std::dynamic_pointer_cast<DUOXCrypto>(crypto);

    EXCEPTION_ASSERT_WITH_LOG(duoxCrypto != nullptr,
        LibLogicalAccessException, "DUOX EV2 AuthenticateEV2First requires the existing DUOXCrypto implementation.");

    EXCEPTION_ASSERT_WITH_LOG(key != nullptr,
        LibLogicalAccessException, "DUOX EV2 AuthenticateEV2First requires an authentication key.");

    const DESFireKeyType keyType = key->getKeyType();

    EXCEPTION_ASSERT_WITH_LOG(keyType == DF_KEY_AES || keyType == DF_KEY_AES256,
        LibLogicalAccessException, "DUOX AuthenticateEV2First only supports AES-128 and AES-256 keys.");

    const std::size_t expectedKeyLength = keyType == DF_KEY_AES ? DESFIRE_AES_KEY_SIZE : DESFIRE_AES256_KEY_SIZE;
    validateAESKeyLength(keyType, key->getLength());

    const std::uint8_t lenCap = static_cast<std::uint8_t>(pcdCap2.size());
    const std::uint8_t keyNo  = buildEV2KeyNo(targetedKeyNo, secondaryApplication);

    ByteVector keyData;

    try
    {
        const ByteVector diversify = getKeyInformations(key, targetedKeyNo);
        duoxCrypto->getKey(key, diversify, keyData);

        EXCEPTION_ASSERT_WITH_LOG(keyData.size() == expectedKeyLength,
            LibLogicalAccessException, "DUOX EV2 diversified authentication key has an invalid length.");

        ByteVector part1Data;
        part1Data.reserve(2U + pcdCap2.size());

        part1Data.push_back(keyNo);
        part1Data.push_back(lenCap);
        if (!pcdCap2.empty())
            part1Data.insert(part1Data.end(), pcdCap2.begin(), pcdCap2.end());

        const auto part1Response = transmit_plain(AUTHENTICATE_EV2_FIRST, part1Data);

        EXCEPTION_ASSERT_WITH_LOG(part1Response.getSW2() == DF_INS_ADDITIONAL_FRAME,
            LibLogicalAccessException, "DUOX AuthenticateEV2First Part 1 failed : expected status 0xAF.");

        EXCEPTION_ASSERT_WITH_LOG(part1Response.getData().size() == EV2_PART1_RESPONSE_SIZE, LibLogicalAccessException,
            "DUOX AuthenticateEV2First Part 1 returned an invalid response length; expected exactly 16 bytes.");

        ByteVector rndB = AESHelper::AESDecrypt(part1Response.getData(), keyData, {});

        EXCEPTION_ASSERT_WITH_LOG(rndB.size() == EV2_RANDOM_SIZE,
            LibLogicalAccessException, "DUOX AuthenticateEV2First Part 1 produced an invalid RndB.");

        ByteVector rndA = RandomHelper::bytes(EV2_RANDOM_SIZE);

        EXCEPTION_ASSERT_WITH_LOG(rndA.size() == EV2_RANDOM_SIZE,
            LibLogicalAccessException, "DUOX EV2 random generator returned an invalid RndA length.");

        const ByteVector rndBPrime = DUOXCrypto::rotateAuthenticationChallenge(rndB);

        ByteVector part2Plaintext;
        part2Plaintext.reserve(EV2_RANDOM_SIZE * 2);

        part2Plaintext.insert(part2Plaintext.end(), rndA.begin(), rndA.end());
        part2Plaintext.insert(part2Plaintext.end(), rndBPrime.begin(), rndBPrime.end());

        EXCEPTION_ASSERT_WITH_LOG(part2Plaintext.size() == EV2_PART2_COMMAND_SIZE, LibLogicalAccessException,
            "Internal DUOX EV2 authentication error: Part 2 plaintext must contain exactly 32 bytes.");

        ByteVector encryptedPart2 = AESHelper::AESEncrypt(part2Plaintext, keyData, {});

        EXCEPTION_ASSERT_WITH_LOG(encryptedPart2.size() == EV2_PART2_COMMAND_SIZE,
            LibLogicalAccessException, "DUOX EV2 AuthenticateEV2First Part 2 encryption returned an invalid length.");

        const auto part2Response = transmit_plain(DF_INS_ADDITIONAL_FRAME, encryptedPart2);

        EXCEPTION_ASSERT_WITH_LOG(part2Response.getSW2() == SW_SUCCESS,
            LibLogicalAccessException, "DUOX AuthenticateEV2First Part 2 failed: expected status 0x00.");

        EXCEPTION_ASSERT_WITH_LOG(part2Response.getData().size() == EV2_PART2_RESPONSE_SIZE, LibLogicalAccessException,
            "DUOX AuthenticateEV2First Part 2 returned an invalid response length; expected exactly 32 bytes.");

        ByteVector decryptedResponse = AESHelper::AESDecrypt(part2Response.getData(), keyData, {});

        EXCEPTION_ASSERT_WITH_LOG(decryptedResponse.size() == EV2_PART2_RESPONSE_SIZE,
            LibLogicalAccessException, "DUOX AuthenticateEV2First Part 2 decrypted response has an invalid length.");

        const ByteVector ti = validateEV2Part2Response(decryptedResponse, rndA, pcdCap2);

        EXCEPTION_ASSERT_WITH_LOG(ti.size() == EV2_TI_SIZE,
            LibLogicalAccessException, "DUOX AuthenticateEV2First returned an invalid transaction identifier.");

        const auto sessionKeys = duoxCrypto->deriveSessionKeys(keyData, rndA, rndB);

        EXCEPTION_ASSERT_WITH_LOG(sessionKeys.first.size() == expectedKeyLength,
            LibLogicalAccessException, "DUOX EV2 authentication produced an invalid encryption session-key length.");

        EXCEPTION_ASSERT_WITH_LOG(sessionKeys.second.size() == expectedKeyLength,
            LibLogicalAccessException,"DUOX EV2 authentication produced an invalid MAC session-key length.");

        duoxCrypto->initializeSecureMessagingState();

        duoxCrypto->setTI(ti);
        duoxCrypto->setCmdCtr(0);
        duoxCrypto->d_sessionKey    = sessionKeys.first;
        duoxCrypto->d_macSessionKey = sessionKeys.second;
        duoxCrypto->d_auth_method   = CryptoMethod::CM_DUOX;
        duoxCrypto->d_currentKeyNo  = targetedKeyNo;

        duoxCrypto->setKey(duoxCrypto->d_currentAid, 0, targetedKeyNo, key);

        clearSensitiveBytes(rndA);
        clearSensitiveBytes(rndB);
        clearSensitiveBytes(part2Plaintext);
        clearSensitiveBytes(encryptedPart2);
        clearSensitiveBytes(decryptedResponse);
        clearSensitiveBytes(keyData);
    }
    catch (...)
    {
        resetDUOXEV2AuthenticationState(duoxCrypto);
        clearSensitiveBytes(keyData);

        throw;
    }
}

// -------------------------------------------------------------------------
// -------------------------------------------------------------------------
// -------------------------------------------------------------------------


// -------------------------------------------------------------------------
// DESFire EV2
// -------------------------------------------------------------------------

// TODO add later EV2 retrocompatibility mode
/*void DUOXISO7816Commands::authenticateEV2First(uint8_t keyno, std::shared_ptr<DESFireKey> key)
{
    DESFireEV3ISO7816Commands::authenticateEV2First(keyno, key);
}*/

void DUOXISO7816Commands::sam_authenticateEV2First(uint8_t keyno, std::shared_ptr<DESFireKey> key)
{
    DESFireEV3ISO7816Commands::sam_authenticateEV2First(keyno, key);
}

void DUOXISO7816Commands::createApplication(
    unsigned int aid, DESFireKeySettings settings, unsigned char maxNbKeys,
    DESFireKeyType cryptoMethod, FidSupport fidSupported, unsigned short isoFID,
    ByteVector isoDFName, unsigned char numberKeySets, unsigned char maxKeySize,
    unsigned char actualkeySetVersion, unsigned char rollkeyno,
    bool specificCapabilityData, bool specificVCKeys)
{
    DESFireEV3ISO7816Commands::createApplication(
        aid, settings, maxNbKeys, cryptoMethod, fidSupported, isoFID, isoDFName,
        numberKeySets, maxKeySize, actualkeySetVersion, rollkeyno, specificCapabilityData,
        specificVCKeys);
}

void DUOXISO7816Commands::createDelegatedApplication(
    std::pair<ByteVector, ByteVector> damInfo, unsigned int aid, unsigned short DAMSlotNo,
    unsigned char DAMSlotVersion, unsigned short quotatLimit, DESFireKeySettings settings,
    unsigned char maxNbKeys, DESFireKeyType cryptoMethod, FidSupport fidSupported,
    unsigned short isoFID, ByteVector isoDFName, unsigned char numberKeySets,
    unsigned char maxKeySize, unsigned char actualkeySetVersion, unsigned char rollkeyno,
    bool specificCapabilityData, bool specificVCKeys)
{
    DESFireEV3ISO7816Commands::createDelegatedApplication(
        damInfo, aid, DAMSlotNo, DAMSlotVersion, quotatLimit, settings, maxNbKeys,
        cryptoMethod, fidSupported, isoFID, isoDFName, numberKeySets, maxKeySize,
        actualkeySetVersion, rollkeyno, specificCapabilityData, specificVCKeys);
}

std::pair<ByteVector, ByteVector> DUOXISO7816Commands::createDAMChallenge(
    std::shared_ptr<DESFireKey> DAMMACKey, std::shared_ptr<DESFireKey> DAMENCKey,
    std::shared_ptr<DESFireKey> DAMDefaultKey, unsigned int aid, unsigned short DAMSlotNo,
    unsigned char DAMSlotVersion, unsigned short quotatLimit, DESFireKeySettings settings,
    unsigned char maxNbKeys, DESFireKeyType cryptoMethod, FidSupport fidSupported,
    unsigned short isoFID, ByteVector isoDFName, unsigned char numberKeySets,
    unsigned char maxKeySize, unsigned char actualkeySetVersion, unsigned char rollkeyno,
    bool specificCapabilityData, bool specificVCKeys)
{
    return DESFireEV3ISO7816Commands::createDAMChallenge(
        DAMMACKey, DAMENCKey, DAMDefaultKey, aid, DAMSlotNo, DAMSlotVersion, quotatLimit,
        settings, maxNbKeys, cryptoMethod, fidSupported, isoFID, isoDFName, numberKeySets,
        maxKeySize, actualkeySetVersion, rollkeyno, specificCapabilityData,
        specificVCKeys);
}

void DUOXISO7816Commands::initializeKeySet(uint8_t keySetNo, DESFireKeyType keySetType)
{
    DESFireEV3ISO7816Commands::initializeKeySet(keySetNo, keySetType);
}

void DUOXISO7816Commands::rollKeySet(uint8_t keySetNo)
{
    DESFireEV3ISO7816Commands::rollKeySet(keySetNo);
}

void DUOXISO7816Commands::finalizeKeySet(uint8_t keySetNo, uint8_t keySetVersion)
{
    DESFireEV3ISO7816Commands::finalizeKeySet(keySetNo, keySetVersion);
}

void DUOXISO7816Commands::createStdDataFile(unsigned char fileno,
                                            EncryptionMode comSettings,
                                            const DESFireAccessRights &accessRights,
                                            unsigned int fileSize, unsigned short isoFID,
                                            bool multiAccessRights)
{
    DESFireEV3ISO7816Commands::createStdDataFile(fileno, comSettings, accessRights,
                                                 fileSize, isoFID, multiAccessRights);
}

void DUOXISO7816Commands::createBackupFile(unsigned char fileno,
                                           EncryptionMode comSettings,
                                           const DESFireAccessRights &accessRights,
                                           unsigned int fileSize, unsigned short isoFID,
                                           bool multiAccessRights)
{
    DESFireEV3ISO7816Commands::createBackupFile(fileno, comSettings, accessRights,
                                                fileSize, isoFID, multiAccessRights);
}

void DUOXISO7816Commands::createLinearRecordFile(
    unsigned char fileno, EncryptionMode comSettings,
    const DESFireAccessRights &accessRights, unsigned int fileSize,
    unsigned int maxNumberOfRecords, unsigned short isoFID, bool multiAccessRights)
{
    DESFireEV3ISO7816Commands::createLinearRecordFile(fileno, comSettings, accessRights,
                                                      fileSize, maxNumberOfRecords,
                                                      isoFID, multiAccessRights);
}

void DUOXISO7816Commands::createCyclicRecordFile(
    unsigned char fileno, EncryptionMode comSettings,
    const DESFireAccessRights &accessRights, unsigned int fileSize,
    unsigned int maxNumberOfRecords, unsigned short isoFID, bool multiAccessRights)
{
    DESFireEV3ISO7816Commands::createCyclicRecordFile(fileno, comSettings, accessRights,
                                                      fileSize, maxNumberOfRecords,
                                                      isoFID, multiAccessRights);
}

std::uint32_t DUOXISO7816Commands::decodeTransactionSignatureCounter(const CommitTransactionResult &result)
{
    EXCEPTION_ASSERT_WITH_LOG(result.type == CommitTransactionResult::Type::Signature, LibLogicalAccessException,
        "Expected Signature result.");

    return decodeTransactionCounter(result.signature.tmc);
}

std::uint32_t DUOXISO7816Commands::decodeTransactionResultCounter(const CommitTransactionResult &result)
{
    switch (result.type)
    {
    case CommitTransactionResult::Type::MAC:
        return decodeTransactionCounter(result.mac.tmc);

    case CommitTransactionResult::Type::Signature:
        return decodeTransactionCounter(result.signature.tmc);

    case CommitTransactionResult::Type::NoCredential:
        throw LibLogicalAccessException("Cannot decode the transaction counter : no transaction credential was returned.");
    }

    THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Cannot decode the transaction counter : invalid credential type.");
}

bool DUOXISO7816Commands::verifyTransactionSignature(const TransactionSignatureFileData &committed,
                                                     EVP_PKEY *applicationPublicKey) const
{
    EXCEPTION_ASSERT_WITH_LOG(hasCommittedTransactionMACInput_,
        LibLogicalAccessException, "Transaction Signature : no successfully committed TMI is available.");

    return DESFireTransactionSignatureCrypto::verifyTSV(
        lastCommittedTransactionMACInput_, committed.tmc,
        committed.tsv, applicationPublicKey);
}

void DUOXISO7816Commands::configureTransactionSignature(unsigned char transactionManagementFileNo)
{
    initializeTransactionMACInputTracking(transactionManagementFileNo);
}

void DUOXISO7816Commands::validateTransactionMACConfiguration(const ByteVector &applicationTMACKey,
                                                              const ByteVector &uid) const
{
    EXCEPTION_ASSERT_WITH_LOG(applicationTMACKey.size() == 16 || applicationTMACKey.size() == 32,
        LibLogicalAccessException, "DUOX TMAC requires a 16 byte AES-128 or 32 byte AES-256 application key.");

    EXCEPTION_ASSERT_WITH_LOG(uid.size() == 7 || uid.size() == 10,
        LibLogicalAccessException, "DUOX TMAC requires a 7 or 10 byte UID.");
}

void DUOXISO7816Commands::createTransactionMACFile(
    unsigned char fileno, EncryptionMode comSettings,
    const DESFireAccessRights &accessRights, std::shared_ptr<DESFireKey> tmkey)
{
    DUOXTransactionMACFileOptions options;
    options.fileNo            = fileno;
    options.communicationMode = comSettings;
    options.accessRights      = accessRights;
    options.mode              = DUOXTransactionMACFileMode::TransactionMAC;
    options.transactionMACKey = std::move(tmkey);

    createTransactionMACFile(options);
}

void DUOXISO7816Commands::createTransactionMACFile(const DUOXTransactionMACFileOptions &options)
{
    const bool transactionSignature = options.mode == DUOXTransactionMACFileMode::TransactionSignature;

    EXCEPTION_ASSERT_WITH_LOG(options.fileNo <= 0x1F,
        LibLogicalAccessException, "Transaction MAC file number must fit in five bits.");

    EXCEPTION_ASSERT_WITH_LOG(options.communicationMode == CM_PLAIN ||
                              options.communicationMode == CM_MAC ||
                              options.communicationMode == CM_ENCRYPT,
        LibLogicalAccessException, "Unsupported communication mode for Transaction MAC file.");

    EXCEPTION_ASSERT_WITH_LOG(options.excludeFilesFromTMI || options.tmiExclusionFileMap == 0,
        LibLogicalAccessException, "A non-zero TMI exclusion map requires exclusion to be enabled.");

    const auto readWriteAccess = options.accessRights.readAndWriteAccess;
    const bool authenticatedCommitReaderID = readWriteAccess != AR_FREE && readWriteAccess != AR_NEVER;

    // Transaction MAC always requires a MAC key. Transaction Signature may include the MAC key parameters.
    const bool includeMACKey = !transactionSignature || authenticatedCommitReaderID;

    unsigned char tmKeyOption = TMAC_KEY_OPTION_NONE;
    ByteVector data;

    if (includeMACKey)
    {
        EXCEPTION_ASSERT_WITH_LOG(options.transactionMACKey != nullptr,
            LibLogicalAccessException, "Transaction MAC key is required for this configuration.");

        const unsigned char keyOption = getTransactionMACKeyOption(options.transactionMACKey);
        tmKeyOption |= keyOption;
        const auto keyData = options.transactionMACKey->getData();
        data.insert(data.end(), keyData.begin(), keyData.end());
        data.push_back(options.transactionMACKey->getKeyVersion());
    }

    if (transactionSignature)
    {
        tmKeyOption |= TMAC_KEY_OPTION_SIGNATURE;

        EXCEPTION_ASSERT_WITH_LOG(options.transactionSignatureKeyNo <= 4,
            LibLogicalAccessException, "DUOX Transaction Signature key number must be between 0 and 4.");
    }

    ByteVector parameters;
    parameters.reserve(options.excludeFilesFromTMI ? 9 : 5);

    parameters.push_back(options.fileNo);
    unsigned char fileOption = static_cast<unsigned char>(options.communicationMode);
    if (options.excludeFilesFromTMI)
        fileOption |= TMAC_FILE_OPTION_TMI_EXCLUSION;
    parameters.push_back(fileOption);
    const short accessRightsValue = AccessRightsInMemory(options.accessRights);
    BufferHelper::setUShort(parameters, accessRightsValue);
    if (options.excludeFilesFromTMI) // Optional DUOX TMI exclusion file map
    {
        for (unsigned int shift = 0; shift < 32; shift += 8)
            parameters.push_back(static_cast<unsigned char>((options.tmiExclusionFileMap >> shift) & 0xFF));
    }
    parameters.push_back(tmKeyOption); // TMKeyOption : key type plus optional Transaction Signature bit
    if (transactionSignature)          // Transaction Signature identifies the ECC private key number
        data.push_back(options.transactionSignatureKeyNo);

    transmit_full(DFEV2_INS_CREATE_TRANSACTION_MAC_FILE, data, parameters);
}

ByteVector DUOXISO7816Commands::getKeyVersion(uint8_t keysetno, uint8_t keyno)
{
    return DESFireEV3ISO7816Commands::getKeyVersion(keysetno, keyno);
}

void DUOXISO7816Commands::setConfiguration(bool formatCardEnabled, bool randomIdEnabled,
                                           bool PCMandatoryEnabled,
                                           bool AuthVCMandatoryEnabled)
{
    DESFireEV3ISO7816Commands::setConfiguration(
        formatCardEnabled, randomIdEnabled, PCMandatoryEnabled, AuthVCMandatoryEnabled);
}

void DUOXISO7816Commands::setConfiguration(uint8_t sak1, uint8_t sak2)
{
    DESFireEV3ISO7816Commands::setConfiguration(sak1, sak2);
}

void DUOXISO7816Commands::setConfiguration(bool D40SecureMessagingEnabled,
                                           bool EV1SecureMessagingEnabled,
                                           bool EV2ChainedWritingEnabled)
{
    DESFireEV3ISO7816Commands::setConfiguration(
        D40SecureMessagingEnabled, EV1SecureMessagingEnabled, EV2ChainedWritingEnabled);
}

void DUOXISO7816Commands::setConfigurationPDCap(uint8_t pdcap1_2, uint8_t pdcap2_5,
                                                uint8_t pdcap2_6)
{
    DESFireEV3ISO7816Commands::setConfigurationPDCap(pdcap1_2, pdcap2_5, pdcap2_6);
}

void DUOXISO7816Commands::setConfiguration(ByteVector DAMMAC, ByteVector ISODFNameOrVCIID)
{
    DESFireEV3ISO7816Commands::setConfiguration(DAMMAC, ISODFNameOrVCIID);
}

void DUOXISO7816Commands::changeFileSettings(
    unsigned char fileno, EncryptionMode comSettings,
    std::vector<DESFireAccessRights> accessRights)
{
    DESFireEV3ISO7816Commands::changeFileSettings(fileno, comSettings, accessRights);
}

void DUOXISO7816Commands::proximityCheck(std::shared_ptr<DESFireKey> key,
                                         uint8_t chunk_size)
{
    DESFireEV3ISO7816Commands::proximityCheck(key, chunk_size);
}

// -------------------------------------------------------------------------
// Transaction management
// -------------------------------------------------------------------------

void DUOXISO7816Commands::abortTransaction()
{
    transmit(DF_INS_ABORT_TRANSACTION); // TODO 0x0C is incorrectly returned as an error; should be a warning
    onTransactionAborted();
}

CommitTransactionResult DUOXISO7816Commands::commitTransaction(bool return_tmac)
{
    using TransactionManagement::TMC_SIZE;
    using TransactionManagement::TMV_SIZE;
    using TransactionManagement::TSV_SIZE;
    using TransactionManagement::TMAC_SIZE;
    using TransactionManagement::TSIG_SIZE;

    constexpr std::uint8_t OPTION_RETURN_TMC = 0x01U;
    constexpr std::uint8_t OPTION_NONE       = 0x00U;

    ByteVector params;
    params.push_back(return_tmac ? OPTION_RETURN_TMC : OPTION_NONE);

    const auto response = transmit(DF_COMMIT_TRANSACTION, params);

    checkSuccess(response, DUOX_SUCCESS, __func__);

    const auto &data = response.getData();

    if (!return_tmac)
    {
        EXCEPTION_ASSERT_WITH_LOG(data.empty(), LibLogicalAccessException,
            "DUOX CommitTransaction returned unexpected response data when no transaction credential was requested.");
        onTransactionCommitted();
        return {};
    }
    
    EXCEPTION_ASSERT_WITH_LOG(data.size() == TMAC_SIZE || data.size() == TSIG_SIZE,
        LibLogicalAccessException, "DUOX CommitTransaction returned an invalid response size : received "
        + std::to_string(data.size()) + " bytes; expected " + std::to_string(TMAC_SIZE) + " bytes (TMC + TMV) or "
        + std::to_string(TSIG_SIZE) + " bytes (TMC + TSV).");

    CommitTransactionResult result;

    if (data.size() == TMAC_SIZE)
    {
        result.type = CommitTransactionResult::Type::MAC;

        std::copy_n(data.begin(), TMC_SIZE, result.mac.tmc.begin());
        std::copy_n(data.begin() + TMC_SIZE, TMV_SIZE, result.mac.tmv.begin());
    }
    else
    {
        result.type = CommitTransactionResult::Type::Signature;

        std::copy_n(data.begin(), TMC_SIZE, result.signature.tmc.begin());
        std::copy_n(data.begin() + TMC_SIZE, TSV_SIZE, result.signature.tsv.begin());
    }

    onTransactionCommitted();

    return result;
}

TransactionMACFileData DUOXISO7816Commands::readTransactionMacFile(unsigned char fileno)
{
    return DESFireEV3ISO7816Commands::readTransactionMacFile(fileno);
}

ByteVector DUOXISO7816Commands::commitReaderID(ByteVector readerid)
{
    return DESFireEV3ISO7816Commands::commitReaderID(readerid);
}
 
void DUOXISO7816Commands::restoreTransfer(unsigned char target_fileno,
                                          unsigned char source_fileno)
{
    DESFireEV3ISO7816Commands::restoreTransfer(target_fileno, source_fileno);
}

ByteVector DUOXISO7816Commands::readSignature(unsigned char address)
{
    return DESFireEV3ISO7816Commands::readSignature(address);
}

bool DUOXISO7816Commands::performECCOriginalityCheck()
{
    return DESFireEV3ISO7816Commands::performECCOriginalityCheck();
}

// -------------------------------------------------------------------------
// DESFire EV3
// -------------------------------------------------------------------------
ByteVector DUOXISO7816Commands::getFileCounters(unsigned char fileno)
{
    return DESFireEV3ISO7816Commands::getFileCounters(fileno);
}

void DUOXISO7816Commands::createStdDataFile(unsigned char fileno, EncryptionMode comSettings,
                       const DESFireAccessRights &accessRights, unsigned int fileSize,
                       unsigned short isoFID, bool multiAccessRights,
                       bool sdmAndMirroring)
{
    DESFireEV3ISO7816Commands::createStdDataFile(fileno, comSettings, accessRights,
                                                 fileSize, isoFID, multiAccessRights,
                                                 sdmAndMirroring);
}

void DUOXISO7816Commands::changeFileSettings(unsigned char fileno, EncryptionMode comSettings,
                        std::vector<DESFireAccessRights> accessRights,
                        bool sdmAndMirroring, unsigned int tmcLimit, bool sdmVCUID,
                        bool sdmReadCtr, bool sdmReadCtrLimit, bool sdmEncFileData,
                        bool asciiEncoding, DESFireAccessRights sdmAccessRights,
                        unsigned int vcuidOffset, unsigned int sdmReadCtrOffset,
                        unsigned int piccDataOffset, unsigned int sdmMacInputOffset,
                        unsigned int sdmEncOffset, unsigned int sdmEncLength,
                        unsigned int sdmMacOffset,
                        unsigned int sdmReadCtrLimitValue)
{
    DESFireEV3ISO7816Commands::changeFileSettings(
        fileno, comSettings, accessRights, sdmAndMirroring, tmcLimit, sdmVCUID,
        sdmReadCtr, sdmReadCtrLimit, sdmEncFileData, asciiEncoding, sdmAccessRights,
        vcuidOffset, sdmReadCtrOffset, piccDataOffset, sdmMacInputOffset, sdmEncOffset,
        sdmEncLength, sdmMacOffset, sdmReadCtrLimitValue);
}

bool DUOXISO7816Commands::performAESOriginalityCheck()
{
    return DESFireEV3ISO7816Commands::performAESOriginalityCheck();
}

void DUOXISO7816Commands::selectApplication(unsigned int aid)
{
    DESFireEV3ISO7816Commands::selectApplication(aid);
}

void DUOXISO7816Commands::selectApplication(unsigned int aid1, unsigned int aid2)
{
    return DESFireEV3ISO7816Commands::selectApplication(aid1, aid2);
}

} // namespace logicalaccess