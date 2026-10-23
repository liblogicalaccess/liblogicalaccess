#include <logicalaccess/plugins/cards/desfire/transactionmaccrypto.hpp>

#include <logicalaccess/myexception.hpp>
#include <logicalaccess/plugins/llacommon/logs.hpp>
#include <logicalaccess/plugins/crypto/aes_helper.hpp>
#include <logicalaccess/plugins/crypto/cmac.hpp>

#include <openssl/crypto.h>

#include <algorithm>
#include <array>
#include <string>

namespace logicalaccess
{

namespace
{

constexpr std::size_t AES_BLOCK_SIZE   = DESFireTransactionMACCrypto::AES_BLOCK_SIZE;
constexpr std::size_t AES_128_KEY_SIZE = DESFireTransactionMACCrypto::AES_128_KEY_SIZE;
constexpr std::size_t AES_256_KEY_SIZE = DESFireTransactionMACCrypto::AES_256_KEY_SIZE;

constexpr std::uint8_t MAC_DERIVATION_LABEL        = 0x5A;
constexpr std::uint8_t ENCRYPTION_DERIVATION_LABEL = 0xA5;

/**
 * \brief Validates UID length compatibility with the shared TMAC crypto.
 */
bool isSupportedUIDSize(const ByteVector &uid)
{
    return uid.size() == 4 || uid.size() == 7 || uid.size() == 10;
}

/**
 * \brief Validates the application TMAC key length and UID length.
 *
 * Supports AES-128 and AES-256 keys and the UID lengths accepted by the shared crypto implementation.
 */
void validateInputs(const ByteVector &key, const ByteVector &uid, const char *function)
{
    EXCEPTION_ASSERT_WITH_LOG(key.size() == AES_128_KEY_SIZE || key.size() == AES_256_KEY_SIZE, LibLogicalAccessException,
        std::string(function) + " : Transaction MAC key must be exactly 16 bytes (AES-128) or 32 bytes (AES-256).");

    EXCEPTION_ASSERT_WITH_LOG(isSupportedUIDSize(uid), LibLogicalAccessException,
        std::string(function) + " : Unsupported UID length. Expected 4, 7 or 10 bytes.");
}

/**
 * \brief Calculates AES-CMAC and validates the resulting block length.
 */
ByteVector calculateAESCMAC(const ByteVector &key, const ByteVector &input)
{
    // Use the library's existing AES-CMAC implementation
    ByteVector result = openssl::CMACCrypto::cmac(key, "aes", input, {});

    EXCEPTION_ASSERT_WITH_LOG(result.size() == AES_BLOCK_SIZE,
        LibLogicalAccessException, "DESFire TMAC : AES-CMAC returned an invalid result length.");

    return result;
}

/**
 * \brief Builds a session-key derivation vector from its label, counter, output length, transaction counter and UID.
 *
 * Zero-pads the complete vector to an AES block boundary when necessary.
 */
ByteVector makeSessionVector(std::uint8_t label, std::uint16_t counter, std::size_t keySize,
                             const std::array<std::uint8_t, DESFireTransactionMACCrypto::TMC_SIZE> &tmc,
                             const ByteVector &uid)
{
    EXCEPTION_ASSERT_WITH_LOG(keySize == AES_128_KEY_SIZE || keySize == AES_256_KEY_SIZE,
        LibLogicalAccessException, "DESFire TMAC : Unsupported session key size.");

    EXCEPTION_ASSERT_WITH_LOG(isSupportedUIDSize(uid),
        LibLogicalAccessException, "DESFire TMAC : Unsupported UID length.");

    const std::uint16_t outputLengthBits = static_cast<std::uint16_t>(keySize * 8);

    // SV : Label || Counter || Length || TMC || UID || ZeroPadding
    ByteVector input;
    input.reserve(AES_BLOCK_SIZE);

    input.push_back(label);
    input.push_back(static_cast<std::uint8_t>((counter >> 8) & 0xFF));
    input.push_back(static_cast<std::uint8_t>(counter & 0xFF));
    input.push_back(static_cast<std::uint8_t>((outputLengthBits >> 8) & 0xFF));
    input.push_back(static_cast<std::uint8_t>(outputLengthBits & 0xFF));
    input.insert(input.end(), tmc.begin(), tmc.end());
    input.insert(input.end(), uid.begin(), uid.end());

    // Append zero bytes only when needed to reach an AES block boundary
    const std::size_t remainder = input.size() % AES_BLOCK_SIZE;

    if (remainder != 0)
        input.resize(input.size() + AES_BLOCK_SIZE - remainder, 0x00);

    EXCEPTION_ASSERT_WITH_LOG(input.size() % AES_BLOCK_SIZE == 0,
        LibLogicalAccessException, "DESFire TMAC : Invalid session vector length.");

    return input;
}

/**
 * \brief Derives an AES session key using the specified derivation label.
 *
 * AES-128 requires one CMAC operation; AES-256 requires two.
 */
ByteVector deriveSessionKey(const ByteVector &applicationTMACKey, std::uint8_t label, std::size_t keySize,
                            const std::array<std::uint8_t, DESFireTransactionMACCrypto::TMC_SIZE> &tmc,
                            const ByteVector &uid)
{
    ByteVector result;
    result.reserve(keySize);

    const std::size_t iterations = keySize / AES_BLOCK_SIZE;

    for (std::size_t i = 0; i < iterations; ++i)
    {
        const auto counter = static_cast<std::uint16_t>(i + 1);
        const ByteVector sessionVector = makeSessionVector(label, counter, keySize, tmc, uid);
        const ByteVector block = calculateAESCMAC(applicationTMACKey, sessionVector);

        EXCEPTION_ASSERT_WITH_LOG(block.size() == AES_BLOCK_SIZE,
            LibLogicalAccessException, "DESFire TMAC : Invalid CMAC output length during session-key derivation.");

        result.insert(result.end(), block.begin(), block.end());
    }

    EXCEPTION_ASSERT_WITH_LOG(result.size() == keySize,
        LibLogicalAccessException, "DESFire TMAC : Derived session key has an invalid length.");

    return result;
}

/**
 * \brief Converts a 16 byte block into a fixed size byte array.
 *
 * Rejects input whose size differs from one AES block.
 */
std::array<std::uint8_t, AES_BLOCK_SIZE> toFixedBlock(const ByteVector &data)
{
    EXCEPTION_ASSERT_WITH_LOG(data.size() == AES_BLOCK_SIZE,
        LibLogicalAccessException, "Expected exactly one 16-byte AES block.");

    std::array<std::uint8_t, AES_BLOCK_SIZE> result{};
    std::copy(data.begin(), data.end(), result.begin());
    return result;
}

} // namespace

TransactionMACSessionKeys DESFireTransactionMACCrypto::deriveAESSessionKeys(
    const ByteVector &applicationTMACKey,
    const std::array<std::uint8_t, TMC_SIZE> &tmcForTransaction, const ByteVector &uid)
{
    validateInputs(applicationTMACKey, uid, __func__);

    const std::size_t keySize = applicationTMACKey.size();

    TransactionMACSessionKeys result;

    result.macKey = deriveSessionKey(applicationTMACKey, MAC_DERIVATION_LABEL, keySize, tmcForTransaction, uid);
    result.encryptionKey = deriveSessionKey(applicationTMACKey, ENCRYPTION_DERIVATION_LABEL, keySize, tmcForTransaction, uid);

    return result;
}

bool DESFireTransactionMACCrypto::verifyTMV(
    const ByteVector &applicationTMACKey,
    const std::array<std::uint8_t, TMC_SIZE> &tmcForTransaction, const ByteVector &uid,
    const ByteVector &tmi, const std::array<std::uint8_t, TMV_SIZE> &expectedTMV)
{
    validateInputs(applicationTMACKey, uid, __func__);

    const auto keys = deriveAESSessionKeys(applicationTMACKey, tmcForTransaction, uid);

    const ByteVector &macKey = keys.macKey;
    const ByteVector fullMAC = calculateAESCMAC(macKey, tmi);

    std::array<std::uint8_t, TMV_SIZE> calculatedTMV{};

    for (std::size_t i = 0; i < TMV_SIZE; ++i)
        calculatedTMV[i] = fullMAC[2 * i + 1];

    return CRYPTO_memcmp(calculatedTMV.data(), expectedTMV.data(), expectedTMV.size()) == 0;
}

std::array<std::uint8_t, DESFireTransactionMACCrypto::READER_ID_SIZE>
DESFireTransactionMACCrypto::decryptPreviousReaderID(
    const ByteVector &applicationTMACKey,
    const std::array<std::uint8_t, TMC_SIZE> &tmcForTransaction, const ByteVector &uid,
    const ByteVector &encryptedReaderID)
{
    validateInputs(applicationTMACKey, uid, __func__);

    EXCEPTION_ASSERT_WITH_LOG(encryptedReaderID.size() == AES_BLOCK_SIZE,
        LibLogicalAccessException, "DESFire TMAC : Encrypted Reader ID must be exactly 16 bytes.");

    const auto keys = deriveAESSessionKeys(applicationTMACKey, tmcForTransaction, uid);

    const ByteVector &encryptionKey = keys.encryptionKey;
    const ByteVector zeroIV(AES_BLOCK_SIZE, 0x00);
    const ByteVector plaintext = AESHelper::AESDecrypt(encryptedReaderID, encryptionKey, zeroIV);

    return toFixedBlock(plaintext);
}

} // namespace logicalaccess