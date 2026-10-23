#include <gtest/gtest.h>

#include <logicalaccess/plugins/cards/desfire/transactionmaccrypto.hpp>
#include <logicalaccess/plugins/crypto/aes_helper.hpp>
#include <logicalaccess/plugins/llacommon/logs.hpp>
#include <logicalaccess/plugins/crypto/cmac.hpp>
#include <logicalaccess/myexception.hpp>

#include <array>
#include <cstdint>

using namespace logicalaccess;

namespace
{

ByteVector makeKey()
{
    return {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
            0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};
}

ByteVector makeAES256Key()
{
    return {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A,
            0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15,
            0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F};
}

ByteVector makeUID()
{
    return {0x04, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66};
}

ByteVector makeUID(std::size_t size)
{
    EXCEPTION_ASSERT_WITH_LOG(size == 4 || size == 7 || size == 10,
        LibLogicalAccessException, "Test UID size must be 4, 7 or 10 bytes.");

    ByteVector uid(size);

    for (std::size_t i = 0; i < size; ++i)
        uid[i] = static_cast<std::uint8_t>(0x04 + i);

    return uid;
}

std::array<std::uint8_t, 4> makeTMC()
{
    return {0x01, 0x00, 0x00, 0x00};
}

} // namespace

TEST(DESFireTransactionMACCryptoTest, DerivesDistinctKeys)
{
    const auto keys = DESFireTransactionMACCrypto::deriveAESSessionKeys(makeKey(), makeTMC(), makeUID());

    EXPECT_NE(keys.macKey, keys.encryptionKey);
}

TEST(DESFireTransactionMACCryptoTest, DerivationIsDeterministic)
{
    const auto key   = makeKey();
    const auto tmc   = makeTMC();
    const auto uid   = makeUID();

    const auto first  = DESFireTransactionMACCrypto::deriveAESSessionKeys(key, tmc, uid);
    const auto second = DESFireTransactionMACCrypto::deriveAESSessionKeys(key, tmc, uid);

    EXPECT_EQ(first.macKey, second.macKey);
    EXPECT_EQ(first.encryptionKey, second.encryptionKey);
}

TEST(DESFireTransactionMACCryptoTest, CounterChangesDerivedKeys)
{
    const auto key = makeKey();
    const auto uid = makeUID();
    const auto tmc = makeTMC();

    auto nextTMC = tmc;
    nextTMC[0]   = 0x02;

    const auto first = DESFireTransactionMACCrypto::deriveAESSessionKeys(key, tmc, uid);
    const auto second = DESFireTransactionMACCrypto::deriveAESSessionKeys(key, nextTMC, uid);

    EXPECT_NE(first.macKey, second.macKey);
    EXPECT_NE(first.encryptionKey, second.encryptionKey);
}

TEST(DESFireTransactionMACCryptoTest, UIDChangesDerivedKeys)
{
    const auto key = makeKey();
    const auto tmc = makeTMC();
    const auto uid = makeUID();

    auto anotherUID = uid;
    anotherUID.back() ^= 0x01;

    const auto first = DESFireTransactionMACCrypto::deriveAESSessionKeys(key, tmc, uid);
    const auto second = DESFireTransactionMACCrypto::deriveAESSessionKeys(key, tmc, anotherUID);

    EXPECT_NE(first.macKey, second.macKey);
    EXPECT_NE(first.encryptionKey, second.encryptionKey);
}

TEST(DESFireTransactionMACCryptoTest, RejectsInvalidKeyLength)
{
    const ByteVector invalidKey(15, 0x00);

    EXPECT_THROW(DESFireTransactionMACCrypto::deriveAESSessionKeys(invalidKey, makeTMC(), makeUID()),
                 LibLogicalAccessException);
}

TEST(DESFireTransactionMACCryptoTest, AcceptsValidUIDLengths)
{
    for (const std::size_t size : {4U, 7U, 10U})
    {
        EXPECT_NO_THROW(DESFireTransactionMACCrypto::deriveAESSessionKeys(makeKey(), makeTMC(), ByteVector(size, 0x00)))
            << "UID size : " << size;
    }
}

TEST(DESFireTransactionMACCryptoTest, RejectsInvalidUIDLengths)
{
    for (const std::size_t size : {0U, 1U, 5U, 6U, 8U, 9U, 11U, 12U})
    {
        EXPECT_THROW(DESFireTransactionMACCrypto::deriveAESSessionKeys(makeKey(), makeTMC(), ByteVector(size, 0x00)),
                     LibLogicalAccessException)
            << "UID size : " << size;
    }
}

TEST(DESFireTransactionMACCryptoTest, RejectsInvalidEncryptedReaderIDLength)
{
    const ByteVector invalidCiphertext(15, 0x00);

    EXPECT_THROW(DESFireTransactionMACCrypto::decryptPreviousReaderID(makeKey(), makeTMC(), makeUID(), invalidCiphertext),
                 LibLogicalAccessException);
}

TEST(DESFireTransactionMACCryptoTest, ReaderIDDecryptRoundTrip)
{
    const auto key = makeKey();
    const auto uid = makeUID();
    const auto tmc = makeTMC();

    const auto keys = DESFireTransactionMACCrypto::deriveAESSessionKeys(key, tmc, uid);

    const ByteVector encryptionKey(keys.encryptionKey.begin(), keys.encryptionKey.end());
    const ByteVector zeroIV(16, 0x00);
    const ByteVector plaintext = {0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7,
                                  0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF};
    const ByteVector ciphertext = AESHelper::AESEncrypt(plaintext, encryptionKey, zeroIV);

    ASSERT_EQ(ciphertext.size(), DESFireTransactionMACCrypto::READER_ID_SIZE);

    const auto decrypted = DESFireTransactionMACCrypto::decryptPreviousReaderID(key, tmc, uid, ciphertext);

    EXPECT_EQ(ByteVector(decrypted.begin(), decrypted.end()), plaintext);
}

TEST(DESFireTransactionMACCryptoTest, VerifiesTMVAndRejectsTampering)
{
    const ByteVector key = makeKey();
    const ByteVector uid = makeUID();
    const auto tmc       = makeTMC();

    // This TMI must represent the exact protocol-defined input, including
    // the zero-padding added after each applicable transaction update
    const ByteVector tmi = {0x3D, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    const auto keys = DESFireTransactionMACCrypto::deriveAESSessionKeys(key, tmc, uid);

    const ByteVector macKey(keys.macKey.begin(), keys.macKey.end());
    const ByteVector zeroIV(16, 0x00);
    const ByteVector fullMAC = openssl::CMACCrypto::cmac(macKey, "aes", tmi, zeroIV);

    ASSERT_EQ(fullMAC.size(), 16U);

    std::array<std::uint8_t, DESFireTransactionMACCrypto::TMV_SIZE> tmv{};

    for (std::size_t i = 0; i < tmv.size(); ++i)
        tmv[i] = fullMAC[2U * i + 1U];

    EXPECT_TRUE(DESFireTransactionMACCrypto::verifyTMV(key, tmc, uid, tmi, tmv));

    tmv[0] ^= 0x01;

    EXPECT_FALSE(DESFireTransactionMACCrypto::verifyTMV(key, tmc, uid, tmi, tmv));
}

TEST(DESFireTransactionMACCryptoTest, AESCMACKnownAnswer)
{
    const ByteVector key = {0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6,
                            0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C};

    const ByteVector message = {0x6B, 0xC1, 0xBE, 0xE2, 0x2E, 0x40, 0x9F, 0x96,
                                0xE9, 0x3D, 0x7E, 0x11, 0x73, 0x93, 0x17, 0x2A};

    const ByteVector expected = {0x07, 0x0A, 0x16, 0xB4, 0x6B, 0x4D, 0x41, 0x44,
                                 0xF7, 0x9B, 0xDD, 0x9D, 0xD0, 0x4A, 0x28, 0x7C};

    const ByteVector zeroIV(16, 0x00);

    const ByteVector actual = openssl::CMACCrypto::cmac(key, "aes", message, zeroIV);

    EXPECT_EQ(actual, expected);
}

TEST(DESFireTransactionMACCryptoTest, SessionKeysMatchReferenceVector)
{
    const ByteVector key = {0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
                            0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F};

    const ByteVector uid = {0x04, 0x6F, 0x48, 0x8A, 0x1C, 0x1E, 0x90};

    const std::array<std::uint8_t, 4> tmc = {0x03, 0x00, 0x00, 0x00};

    const auto keys = DESFireTransactionMACCrypto::deriveAESSessionKeys(key, tmc, uid);

    // Populate only from an independently verified reference
    const ByteVector expectedMACKey = {0xCA, 0x1E, 0x03, 0x5A, 0xAC, 0xCD, 0xAB, 0x85,
                                       0xD5, 0xFE, 0xF3, 0xCE, 0x84, 0x00, 0x6C, 0xD6};
    const ByteVector expectedENCKey = {0x67, 0x3F, 0xAD, 0x03, 0x64, 0xBD, 0x36, 0x17,
                                       0x40, 0xFE, 0xCE, 0xB6, 0x2C, 0x01, 0x99, 0x21};

    EXPECT_EQ(keys.macKey, expectedMACKey);
    EXPECT_EQ(keys.encryptionKey, expectedENCKey);
}

TEST(DESFireTransactionMACCryptoTest, FullCMACAndTMVMatchReferenceVector)
{
    const ByteVector key = {0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
                            0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F};

    const ByteVector uid = {0x04, 0x6F, 0x48, 0x8A, 0x1C, 0x1E, 0x90};

    const std::array<std::uint8_t, 4> tmc = {0x04, 0x00, 0x00, 0x00};

    /// Deterministic 80 byte TMI from real hardware test.
    /// Provides regression coverage for Transaction MAC verification against actual DESFire protocol behavior.
    const ByteVector tmi = {0xC8, 0x28, 0xBF, 0x19, 0x82, 0xBE, 0x08, 0x6F, 0xBC, 0x60,
                            0xA2, 0x2D, 0xAE, 0xB6, 0x66, 0x13, 0xEE, 0x00, 0x00, 0x00,
                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                            0x00, 0x00, 0x3D, 0x01, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00,
                            0xDE, 0xAD, 0xBE, 0xEF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    ASSERT_EQ(tmi.size(), 80U);
    ASSERT_EQ(tmi.size() % 16, 0U);

    const auto keys = DESFireTransactionMACCrypto::deriveAESSessionKeys(key, tmc, uid);

    const ByteVector macKey(keys.macKey.begin(), keys.macKey.end());
    const ByteVector zeroIV(16, 0x00);
    const ByteVector fullMAC = openssl::CMACCrypto::cmac(macKey, "aes", tmi, zeroIV);

    ASSERT_EQ(fullMAC.size(), 16U);

    std::array<std::uint8_t, DESFireTransactionMACCrypto::TMV_SIZE> expectedTMV{};

    // Use the same truncation convention as verifyTMV()
    for (std::size_t i = 0; i < expectedTMV.size(); ++i)
        expectedTMV[i] = fullMAC[2U * i + 1U];

    EXPECT_TRUE(DESFireTransactionMACCrypto::verifyTMV(key, tmc, uid, tmi, expectedTMV));

    auto tamperedTMV = expectedTMV;
    tamperedTMV[0] ^= 0x01;

    EXPECT_FALSE(DESFireTransactionMACCrypto::verifyTMV(key, tmc, uid, tmi, tamperedTMV));
}

TEST(DESFireTransactionMACCryptoTest, CounterByteOrderAffectsDerivedKeys)
{
    auto tmc1 = std::array<std::uint8_t, 4>{0xFF, 0x00, 0x00, 0x00};
    auto tmc2 = std::array<std::uint8_t, 4>{0x00, 0x01, 0x00, 0x00};

    const auto first = DESFireTransactionMACCrypto::deriveAESSessionKeys(makeKey(), tmc1, makeUID());
    const auto second = DESFireTransactionMACCrypto::deriveAESSessionKeys(makeKey(), tmc2, makeUID());

    EXPECT_NE(first.macKey, second.macKey);
    EXPECT_NE(first.encryptionKey, second.encryptionKey);
}

TEST(DESFireTransactionMACCryptoTest, AES256Derives32ByteSessionKeys)
{
    const auto keys = DESFireTransactionMACCrypto::deriveAESSessionKeys(makeAES256Key(), makeTMC(), makeUID());

    ASSERT_EQ(keys.macKey.size(), 32U);
    ASSERT_EQ(keys.encryptionKey.size(), 32U);

    EXPECT_NE(keys.macKey, keys.encryptionKey);
}

TEST(DESFireTransactionMACCryptoTest, AES256DerivationIsDeterministic)
{
    const auto key = makeAES256Key();
    const auto tmc = makeTMC();
    const auto uid = makeUID();

    const auto first = DESFireTransactionMACCrypto::deriveAESSessionKeys(key, tmc, uid);
    const auto second = DESFireTransactionMACCrypto::deriveAESSessionKeys(key, tmc, uid);

    EXPECT_EQ(first.macKey, second.macKey);
    EXPECT_EQ(first.encryptionKey, second.encryptionKey);
}

TEST(DESFireTransactionMACCryptoTest, AES256CounterChangesDerivedKeys)
{
    const auto key = makeAES256Key();
    const auto uid = makeUID();
    const auto tmc = makeTMC();

    auto nextTMC = tmc;
    nextTMC[0] ^= 0x01;

    const auto first = DESFireTransactionMACCrypto::deriveAESSessionKeys(key, tmc, uid);
    const auto second = DESFireTransactionMACCrypto::deriveAESSessionKeys(key, nextTMC, uid);

    EXPECT_NE(first.macKey, second.macKey);
    EXPECT_NE(first.encryptionKey, second.encryptionKey);
}

TEST(DESFireTransactionMACCryptoTest, AES256UIDChangesDerivedKeys)
{
    const auto key = makeAES256Key();
    const auto tmc = makeTMC();
    const auto uid = makeUID();

    auto anotherUID = uid;
    anotherUID.back() ^= 0x01;

    const auto first = DESFireTransactionMACCrypto::deriveAESSessionKeys(key, tmc, uid);
    const auto second = DESFireTransactionMACCrypto::deriveAESSessionKeys(key, tmc, anotherUID);

    EXPECT_NE(first.macKey, second.macKey);
    EXPECT_NE(first.encryptionKey, second.encryptionKey);
}

TEST(DESFireTransactionMACCryptoTest, AES256ReaderIDDecryptRoundTrip)
{
    const auto key = makeAES256Key();
    const auto uid = makeUID();
    const auto tmc = makeTMC();

    const auto keys = DESFireTransactionMACCrypto::deriveAESSessionKeys(key, tmc, uid);

    ASSERT_EQ(keys.encryptionKey.size(), 32U);

    const ByteVector zeroIV(16, 0x00);

    const ByteVector plaintext = {0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7,
                                  0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF};

    const ByteVector ciphertext = AESHelper::AESEncrypt(plaintext, keys.encryptionKey, zeroIV);

    ASSERT_EQ(ciphertext.size(), DESFireTransactionMACCrypto::READER_ID_SIZE);

    const auto decrypted = DESFireTransactionMACCrypto::decryptPreviousReaderID(key, tmc, uid, ciphertext);

    EXPECT_EQ(ByteVector(decrypted.begin(), decrypted.end()), plaintext);
}

TEST(DESFireTransactionMACCryptoTest, AES256VerifiesTMVAndRejectsTampering)
{
    const auto key = makeAES256Key();
    const auto uid = makeUID();
    const auto tmc = makeTMC();

    const ByteVector tmi = {0x3D, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    const auto keys = DESFireTransactionMACCrypto::deriveAESSessionKeys(key, tmc, uid);

    ASSERT_EQ(keys.macKey.size(), 32U);

    const ByteVector zeroIV(16, 0x00);
    const ByteVector fullMAC = openssl::CMACCrypto::cmac(keys.macKey, "aes", tmi, zeroIV);

    ASSERT_EQ(fullMAC.size(), 16U);

    std::array<std::uint8_t, DESFireTransactionMACCrypto::TMV_SIZE> tmv{};

    for (std::size_t i = 0; i < tmv.size(); ++i)
        tmv[i] = fullMAC[2U * i + 1U];

    EXPECT_TRUE(DESFireTransactionMACCrypto::verifyTMV(key, tmc, uid, tmi, tmv));

    tmv[0] ^= 0x01;

    EXPECT_FALSE(DESFireTransactionMACCrypto::verifyTMV(key, tmc, uid, tmi, tmv));
}

TEST(DESFireTransactionMACCryptoTest, AES256CMACKnownAnswer)
{
    const ByteVector key = {0x60, 0x3D, 0xEB, 0x10, 0x15, 0xCA, 0x71, 0xBE,
                            0x2B, 0x73, 0xAE, 0xF0, 0x85, 0x7D, 0x77, 0x81,
                            0x1F, 0x35, 0x2C, 0x07, 0x3B, 0x61, 0x08, 0xD7,
                            0x2D, 0x98, 0x10, 0xA3, 0x09, 0x14, 0xDF, 0xF4};

    const ByteVector emptyMessage;
    const ByteVector zeroIV(16, 0x00);

    const ByteVector expected = {0x02, 0x89, 0x62, 0xF6, 0x1B, 0x7B, 0xF8, 0x9E,
                                 0xFC, 0x6B, 0x55, 0x1F, 0x46, 0x67, 0xD9, 0x83};

    const ByteVector actual = openssl::CMACCrypto::cmac(key, "aes", emptyMessage, zeroIV);

    EXPECT_EQ(actual, expected);
}

TEST(DESFireTransactionMACCryptoTest, RejectsAES192KeyLength)
{
    const ByteVector invalidKey(24, 0x00);

    EXPECT_THROW(DESFireTransactionMACCrypto::deriveAESSessionKeys(invalidKey, makeTMC(), makeUID()),
        LibLogicalAccessException);
}

TEST(DESFireTransactionMACCryptoTest, RejectsOversizedKeyLength)
{
    const ByteVector invalidKey(33, 0x00);

    EXPECT_THROW(DESFireTransactionMACCrypto::deriveAESSessionKeys(invalidKey, makeTMC(), makeUID()),
        LibLogicalAccessException);
}

TEST(DESFireTransactionMACCryptoTest, SupportsAllValidUIDLengthsAES128)
{
    const auto key = makeKey();
    const auto tmc = makeTMC();

    for (const std::size_t uidSize : {4U, 7U, 10U})
    {
        const auto uid = makeUID(uidSize);
        const auto keys = DESFireTransactionMACCrypto::deriveAESSessionKeys(key, tmc, uid);

        EXPECT_EQ(keys.macKey.size(), 16U) << "UID size : " << uidSize;
        EXPECT_EQ(keys.encryptionKey.size(), 16U) << "UID size : " << uidSize;

        EXPECT_NE(keys.macKey, keys.encryptionKey) << "UID size : " << uidSize;
    }
}

TEST(DESFireTransactionMACCryptoTest, SupportsAllValidUIDLengthsAES256)
{
    const auto key = makeAES256Key();
    const auto tmc = makeTMC();

    for (const std::size_t uidSize : {4U, 7U, 10U})
    {
        const auto uid = makeUID(uidSize);
        const auto keys = DESFireTransactionMACCrypto::deriveAESSessionKeys(key, tmc, uid);

        EXPECT_EQ(keys.macKey.size(), 32U) << "UID size : " << uidSize;
        EXPECT_EQ(keys.encryptionKey.size(), 32U) << "UID size : " << uidSize;

        EXPECT_NE(keys.macKey, keys.encryptionKey) << "UID size : " << uidSize;
    }
}

TEST(DESFireTransactionMACCryptoTest, DifferentUIDLengthsProduceDifferentKeys)
{
    const auto key = makeKey();
    const auto tmc = makeTMC();

    const auto keys4 = DESFireTransactionMACCrypto::deriveAESSessionKeys(key, tmc, makeUID(4));
    const auto keys7 = DESFireTransactionMACCrypto::deriveAESSessionKeys(key, tmc, makeUID(7));
    const auto keys10 = DESFireTransactionMACCrypto::deriveAESSessionKeys(key, tmc, makeUID(10));

    EXPECT_NE(keys4.macKey, keys7.macKey);
    EXPECT_NE(keys4.macKey, keys10.macKey);
    EXPECT_NE(keys7.macKey, keys10.macKey);

    EXPECT_NE(keys4.encryptionKey, keys7.encryptionKey);
    EXPECT_NE(keys4.encryptionKey, keys10.encryptionKey);
    EXPECT_NE(keys7.encryptionKey, keys10.encryptionKey);
}

TEST(DESFireTransactionMACCryptoTest, RejectsUnsupportedUIDLengths)
{
    const auto key = makeKey();
    const auto tmc = makeTMC();

    for (const std::size_t uidSize : {0U, 1U, 5U, 6U, 8U, 9U, 11U, 12U})
    {
        const ByteVector uid(uidSize, 0x00);

        EXPECT_THROW(DESFireTransactionMACCrypto::deriveAESSessionKeys(key, tmc, uid), LibLogicalAccessException)
            << "UID size : " << uidSize;
    }
}

TEST(DESFireTransactionMACCryptoTest, VerifyTMVSupportsAllValidUIDLengths)
{
    const auto key = makeKey();
    const auto tmc = makeTMC();

    const ByteVector tmi(16, 0x00);
    const std::array<std::uint8_t, DESFireTransactionMACCrypto::TMV_SIZE> tmv{};

    for (const std::size_t uidSize : {4U, 7U, 10U})
    {
        EXPECT_NO_THROW(DESFireTransactionMACCrypto::verifyTMV(key, tmc, makeUID(uidSize), tmi, tmv))
            << "UID size : " << uidSize;
    }
}

TEST(DESFireTransactionMACCryptoTest, DecryptReaderIDSupportsAllValidUIDLengths)
{
    const auto key = makeKey();
    const auto tmc = makeTMC();

    const ByteVector ciphertext(16, 0x00);

    for (const std::size_t uidSize : {4U, 7U, 10U})
    {
        EXPECT_NO_THROW(DESFireTransactionMACCrypto::decryptPreviousReaderID(key, tmc, makeUID(uidSize), ciphertext))
            << "UID size : " << uidSize;
    }
}