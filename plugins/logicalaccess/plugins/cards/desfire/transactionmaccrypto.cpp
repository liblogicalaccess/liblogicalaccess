#include <logicalaccess/plugins/cards/desfire/transactionmaccrypto.hpp>

#include <logicalaccess/plugins/crypto/secure_memory.hpp>
#include <logicalaccess/plugins/crypto/aes_helper.hpp>
#include <logicalaccess/plugins/crypto/cmac.hpp>

#include <openssl/crypto.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <string>

#include <logicalaccess/myexception.hpp>
#include <logicalaccess/plugins/llacommon/logs.hpp>

namespace logicalaccess
{

namespace
{

constexpr std::uint8_t MAC_DERIVATION_LABEL        = 0x5A;
constexpr std::uint8_t ENCRYPTION_DERIVATION_LABEL = 0xA5;

/**
 * \brief Validates UID length compatibility with the shared TMAC crypto.
 */
bool isSupportedUIDSize(const ByteVector &uid)
{
    return uid.size() == 4U || uid.size() == 7U || uid.size() == 10U;
}

/**
 * \brief Returns true if the key size is supported (AES-128 or AES-256).
 */
bool isSupportedAESKeySize(std::size_t keySize) noexcept
{
    return keySize == TransactionManagement::AES_128_KEY_SIZE || keySize == TransactionManagement::AES_256_KEY_SIZE;
}

/**
 * \brief Validates the application TMAC key length and UID length.
 *
 * Supports AES-128 and AES-256 keys and the UID lengths accepted by the shared crypto implementation.
 */
void validateInputs(const ByteVector &key, const ByteVector &uid, const char *function)
{
    EXCEPTION_ASSERT_WITH_LOG(isSupportedAESKeySize(key.size()), LibLogicalAccessException,
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

    EXCEPTION_ASSERT_WITH_LOG(result.size() == TransactionManagement::AES_BLOCK_SIZE,
        LibLogicalAccessException, "DESFire TMAC : AES-CMAC returned an invalid result length.");

    return result;
}

/**
 * \brief Builds a session-key derivation vector from its label, counter, output length, transaction counter and UID.
 *
 * Zero-pads the complete vector to an AES block boundary when necessary.
 */
ByteVector makeSessionVector(std::uint8_t label, std::uint16_t counter, std::size_t keySize,
                             const TransactionManagement::TransactionCounter &tmc, const ByteVector &uid)
{
    const std::uint16_t outputLengthBits = static_cast<std::uint16_t>(keySize * 8U);
    
    const std::size_t unpaddedSize = 5U + TransactionManagement::TMC_SIZE + uid.size();
    const std::size_t paddingSize = (TransactionManagement::AES_BLOCK_SIZE -
        (unpaddedSize % TransactionManagement::AES_BLOCK_SIZE)) % TransactionManagement::AES_BLOCK_SIZE;

    // SV : Label || Counter || Length || TMC || UID || ZeroPadding
    ByteVector input;
    input.reserve(unpaddedSize + paddingSize);

    input.push_back(label);
    input.push_back(static_cast<std::uint8_t>((counter >> 8U) & 0xFFU));
    input.push_back(static_cast<std::uint8_t>(counter & 0xFFU));
    input.push_back(static_cast<std::uint8_t>((outputLengthBits >> 8U) & 0xFFU));
    input.push_back(static_cast<std::uint8_t>(outputLengthBits & 0xFFU));
    input.insert(input.end(), tmc.begin(), tmc.end());
    input.insert(input.end(), uid.begin(), uid.end());

    // Append zero bytes to align the session vector to an AES block boundary.
    // A 7 byte UID requires no padding; other supported UID lengths do.
    input.resize(input.size() + paddingSize, 0x00U);

    EXCEPTION_ASSERT_WITH_LOG(input.size() % TransactionManagement::AES_BLOCK_SIZE == 0U,
        LibLogicalAccessException, "DESFire TMAC : Invalid session vector length.");

    return input;
}

/**
 * \brief Derives an AES session key using the specified derivation label.
 *
 * AES-128 requires one CMAC operation; AES-256 requires two.
 */
ByteVector deriveSessionKey(const ByteVector &applicationTMACKey, std::uint8_t label, std::size_t keySize,
                            const TransactionManagement::TransactionCounter &tmc, const ByteVector &uid)
{
    ByteVector result;
    result.reserve(keySize);

    const std::size_t iterations = keySize / TransactionManagement::AES_BLOCK_SIZE;

    for (std::size_t i = 0; i < iterations; ++i)
    {
        const auto counter = static_cast<std::uint16_t>(i + 1U);
        ByteVector sessionVector = makeSessionVector(label, counter, keySize, tmc, uid);
        ByteVector block;
        // Install guards before cryptographic operation so that exceptions cannot bypass cleanup
        security::SecureZeroGuard cleanupGuard{&sessionVector, &block};
        block = calculateAESCMAC(applicationTMACKey, sessionVector);

        EXCEPTION_ASSERT_WITH_LOG(block.size() == TransactionManagement::AES_BLOCK_SIZE,
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
std::array<std::uint8_t, TransactionManagement::AES_BLOCK_SIZE> toFixedBlock(const ByteVector &data)
{
    EXCEPTION_ASSERT_WITH_LOG(data.size() == TransactionManagement::AES_BLOCK_SIZE,
        LibLogicalAccessException, "Expected exactly one 16-byte AES block.");

    std::array<std::uint8_t, TransactionManagement::AES_BLOCK_SIZE> result{};
    std::copy(data.begin(), data.end(), result.begin());
    return result;
}

} // namespace

TransactionMACSessionKeys DESFireTransactionMACCrypto::deriveAESSessionKeys(
    const ByteVector &applicationTMACKey, const TransactionManagement::TransactionCounter &tmcForTransaction,
    const ByteVector &uid)
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
    const TransactionManagement::TransactionCounter &tmcForTransaction,
    const ByteVector &uid, const ByteVector &tmi,
    const TransactionManagement::TransactionMACValue &expectedTMV)
{
    validateInputs(applicationTMACKey, uid, __func__);

    EXCEPTION_ASSERT_WITH_LOG(tmi.size() % TransactionManagement::AES_BLOCK_SIZE == 0U,
        LibLogicalAccessException, "DESFire TMAC : TMI must be block-aligned.");

    auto keys = deriveAESSessionKeys(applicationTMACKey, tmcForTransaction, uid);
    security::SecureZeroGuard keyGuard{&keys.macKey, &keys.encryptionKey};

    ByteVector fullMAC = calculateAESCMAC(keys.macKey, tmi);
    security::SecureZeroGuard macGuard{&fullMAC};

    TransactionManagement::TransactionMACValue calculatedTMV{};

    for (std::size_t i = 0; i < TransactionManagement::TMV_SIZE; ++i)
        calculatedTMV[i] = fullMAC[2U * i + 1U];

    return CRYPTO_memcmp(calculatedTMV.data(), expectedTMV.data(), expectedTMV.size()) == 0;
}

TransactionManagement::ReaderID DESFireTransactionMACCrypto::decryptPreviousReaderID(
    const ByteVector &applicationTMACKey,
    const TransactionManagement::TransactionCounter &tmcForTransaction,
    const ByteVector &uid,
    const ByteVector &encryptedReaderID)
{
    validateInputs(applicationTMACKey, uid, __func__);

    EXCEPTION_ASSERT_WITH_LOG(encryptedReaderID.size() == TransactionManagement::AES_BLOCK_SIZE,
        LibLogicalAccessException, "DESFire TMAC : Encrypted Reader ID must be exactly 16 bytes.");

    auto keys = deriveAESSessionKeys(applicationTMACKey, tmcForTransaction, uid);
    security::SecureZeroGuard keyGuard{&keys.macKey, &keys.encryptionKey};

    const ByteVector zeroIV(TransactionManagement::AES_BLOCK_SIZE, 0x00U);
    ByteVector plaintext = AESHelper::AESDecrypt(encryptedReaderID, keys.encryptionKey, zeroIV);
    security::SecureZeroGuard plaintextGuard{&plaintext};

    return toFixedBlock(plaintext);
}

} // namespace logicalaccess