#include <logicalaccess/plugins/cards/duox/duoxcrypto.hpp>

#include <algorithm>
#include <cstdint>
#include <memory>
#include <utility>

#include <logicalaccess/myexception.hpp>
#include <logicalaccess/plugins/cards/desfire/desfirecommands.hpp>
#include <logicalaccess/plugins/crypto/aes_helper.hpp>
#include <logicalaccess/plugins/crypto/aes_symmetric_key.hpp>
#include <logicalaccess/plugins/crypto/cmac.hpp>
#include <logicalaccess/plugins/crypto/secure_memory.hpp>

namespace logicalaccess
{

namespace
{
constexpr std::size_t EV2_RANDOM_SIZE = 16U;

constexpr std::size_t AES128_KEY_SIZE = DESFIRE_AES_KEY_SIZE;
constexpr std::size_t AES256_KEY_SIZE = DESFIRE_AES256_KEY_SIZE;

constexpr std::uint16_t AES128_KDF_LENGTH = 0x0080U;
constexpr std::uint16_t AES256_KDF_LENGTH = 0x0100U;

constexpr std::uint8_t ENC_LABEL_0 = 0xA5U;
constexpr std::uint8_t ENC_LABEL_1 = 0x5AU;

constexpr std::uint8_t MAC_LABEL_0 = 0x5AU;
constexpr std::uint8_t MAC_LABEL_1 = 0xA5U;

constexpr std::size_t KDF_CONTEXT_SIZE    = 26U;
constexpr std::size_t KDF_VECTOR_SIZE     = 32U;

} // namespace

ByteVector DUOXCrypto::rotateAuthenticationChallenge(const ByteVector &rnd)
{
    EXCEPTION_ASSERT_WITH_LOG(
        rnd.size() == EV2_RANDOM_SIZE, LibLogicalAccessException,
        "DUOX EV2 authentication challenge must contain exactly 16 bytes.");

    ByteVector rotated = rnd;
    std::rotate(rotated.begin(), rotated.begin() + 1, rotated.end());

    return rotated;
}

ByteVector DUOXCrypto::buildKDFContext(const ByteVector &rndA, const ByteVector &rndB)
{
    constexpr std::size_t EV2_CONTEXT_XOR_SIZE         = 6U;
    constexpr std::size_t EV2_CONTEXT_RNDB_OFFSET      = 6U;
    constexpr std::size_t EV2_CONTEXT_RNDA_OFFSET      = 8U;
    constexpr std::size_t KDF_CONTEXT_RNDA_PREFIX_SIZE = 2U;

    EXCEPTION_ASSERT_WITH_LOG(rndA.size() == EV2_RANDOM_SIZE,
        LibLogicalAccessException, "DUOX EV2 RndA must contain exactly 16 bytes.");

    EXCEPTION_ASSERT_WITH_LOG(rndB.size() == EV2_RANDOM_SIZE,
        LibLogicalAccessException, "DUOX EV2 RndB must contain exactly 16 bytes.");

    ByteVector context;
    context.reserve(KDF_CONTEXT_SIZE);

    // RndA[0...1]
    context.insert(context.end(), rndA.begin(), rndA.begin() + KDF_CONTEXT_RNDA_PREFIX_SIZE);

    for (std::size_t i = 0; i < EV2_CONTEXT_XOR_SIZE; ++i)
        context.push_back(static_cast<std::uint8_t>(rndA[2U + i] ^ rndB[i]));

    context.insert(context.end(), rndB.begin() + EV2_CONTEXT_RNDB_OFFSET, rndB.end()); // RndB[6...15]
    context.insert(context.end(), rndA.begin() + EV2_CONTEXT_RNDA_OFFSET, rndA.end()); // RndA[8...15]

    EXCEPTION_ASSERT_WITH_LOG(context.size() == KDF_CONTEXT_SIZE,
        LibLogicalAccessException, "Internal DUOX EV2 error: invalid KDF context size.");

    return context;
}

ByteVector DUOXCrypto::buildKDFVector(std::uint8_t label0, std::uint8_t label1,
                                      std::uint16_t counter, std::uint16_t length,
                                      const ByteVector &context)
{
    EXCEPTION_ASSERT_WITH_LOG(context.size() == KDF_CONTEXT_SIZE,
        LibLogicalAccessException, "DUOX EV2 KDF context must contain exactly 26 bytes.");

    EXCEPTION_ASSERT_WITH_LOG(counter != 0U,
        LibLogicalAccessException, "DUOX EV2 KDF counter must be non-zero.");

    ByteVector vector;
    vector.reserve(KDF_VECTOR_SIZE);

    vector.push_back(label0);
    vector.push_back(label1);
    vector.push_back(static_cast<std::uint8_t>((counter >> 8U) & 0xFFU));
    vector.push_back(static_cast<std::uint8_t>(counter & 0xFFU));
    vector.push_back(static_cast<std::uint8_t>((length >> 8U) & 0xFFU));
    vector.push_back(static_cast<std::uint8_t>(length & 0xFFU));
    vector.insert(vector.end(), context.begin(), context.end());

    EXCEPTION_ASSERT_WITH_LOG(vector.size() == KDF_VECTOR_SIZE,
        LibLogicalAccessException, "Internal DUOX EV2 error : KDF vector must contain exactly 32 bytes.");

    return vector;
}

std::pair<ByteVector, ByteVector> DUOXCrypto::deriveSessionKeys(
    const ByteVector &key, const ByteVector &rndA, const ByteVector &rndB)
{
    constexpr std::size_t KDF_CMAC_BLOCK_SIZE  = 16U;
    constexpr std::uint16_t KDF_COUNTER_FIRST  = 0x0001U;
    constexpr std::uint16_t KDF_COUNTER_SECOND = 0x0002U;

    const bool aes128 = key.size() == AES128_KEY_SIZE;
    const bool aes256 = key.size() == AES256_KEY_SIZE;

    EXCEPTION_ASSERT_WITH_LOG(aes128 || aes256,
        LibLogicalAccessException, "DUOX EV2 authentication requires an AES-128 or AES-256 key.");

    const ByteVector context = buildKDFContext(rndA, rndB);
    const std::uint16_t kdfLength = aes128 ? AES128_KDF_LENGTH : AES256_KDF_LENGTH;

    const auto deriveBlock = [&](std::uint8_t label0, std::uint8_t label1, std::uint16_t counter) -> ByteVector
    {
        const ByteVector kdfVector = buildKDFVector(label0, label1, counter, kdfLength, context);
        const ByteVector block = openssl::CMACCrypto::cmac(key, "aes", kdfVector, {});

        EXCEPTION_ASSERT_WITH_LOG(block.size() == KDF_CMAC_BLOCK_SIZE,
            LibLogicalAccessException, "DUOX EV2 KDF CMAC returned an invalid 16-byte block.");

        return block;
    };

    ByteVector sessionEnc = deriveBlock(ENC_LABEL_0, ENC_LABEL_1, KDF_COUNTER_FIRST);
    ByteVector sessionMac = deriveBlock(MAC_LABEL_0, MAC_LABEL_1, KDF_COUNTER_FIRST);

    if (aes128)
    {
        EXCEPTION_ASSERT_WITH_LOG(sessionEnc.size() == AES128_KEY_SIZE && sessionMac.size() == AES128_KEY_SIZE,
            LibLogicalAccessException, "DUOX EV2 AES-128 KDF produced invalid session-key lengths.");

        return {std::move(sessionEnc), std::move(sessionMac)};
    }

    const ByteVector enc2 = deriveBlock(ENC_LABEL_0, ENC_LABEL_1, KDF_COUNTER_SECOND);
    const ByteVector mac2 = deriveBlock(MAC_LABEL_0, MAC_LABEL_1, KDF_COUNTER_SECOND);

    sessionEnc.insert(sessionEnc.end(), enc2.begin(), enc2.end());
    sessionMac.insert(sessionMac.end(), mac2.begin(), mac2.end());

    EXCEPTION_ASSERT_WITH_LOG(sessionEnc.size() == AES256_KEY_SIZE && sessionMac.size() == AES256_KEY_SIZE,
        LibLogicalAccessException, "DUOX EV2 AES-256 KDF produced invalid session-key lengths.");

    return {std::move(sessionEnc), std::move(sessionMac)};
}

ByteVector DUOXCrypto::buildChangeKeyData(std::uint8_t keySetNo, std::uint8_t keyNo,
    const ByteVector &oldKeyDiversify, const std::shared_ptr<DESFireKey> newKey, const ByteVector &newKeyDiversify)
{
    constexpr std::uint8_t KEYNO_MASK = 0x3FU;

    EXCEPTION_ASSERT_WITH_LOG(newKey != nullptr,
        LibLogicalAccessException, "DUOX key change requires a valid new key.");

    const auto keyType = newKey->getKeyType();

    // DUOX ChangeKey supports AES keys only. AES-128 or AES-256 is determined by the actual key length
    EXCEPTION_ASSERT_WITH_LOG(keyType == DESFireKeyType::DF_KEY_AES || keyType == DESFireKeyType::DF_KEY_AES256,
        LibLogicalAccessException, "DUOX key change requires an AES-128 or AES-256 key.");

    ByteVector newKeyData;
    getKey(newKey, newKeyDiversify, newKeyData);

    const auto diversifiedKeySize = newKeyData.size();

    EXCEPTION_ASSERT_WITH_LOG((keyType == DESFireKeyType::DF_KEY_AES && diversifiedKeySize == DESFIRE_AES_KEY_SIZE) ||
            (keyType == DESFireKeyType::DF_KEY_AES256 && diversifiedKeySize == DESFIRE_AES256_KEY_SIZE),
        LibLogicalAccessException, "DUOX key change produced an invalid diversified AES key length.");

    const std::uint8_t targetKeyNo = static_cast<std::uint8_t>(keyNo & KEYNO_MASK);

    const bool changingAuthenticatedKey = keySetNo == 0x00 && targetKeyNo == d_currentKeyNo;

    ByteVector keyData;

    if (changingAuthenticatedKey)
    {
        // When changing the currently authenticated key, DUOX expects the new key directly followed by its key version
        keyData.reserve(newKeyData.size() + 1U);
        keyData.insert(keyData.end(), newKeyData.begin(), newKeyData.end());
        keyData.push_back(newKey->getKeyVersion());
    }
    else
    {
        ByteVector oldKey;
        const bool oldKeyAvailable = getKey(keySetNo, targetKeyNo, oldKeyDiversify, oldKey);

        // A disabled/uninitialized target key is represented by an all-zero OldKey
        // Otherwise, adapt OldKey to the new key size
        if (!oldKeyAvailable)
        {
            oldKey.assign(newKeyData.size(), 0x00U);
        }
        else
        {
            oldKey.resize(newKeyData.size(), 0x00U);
        }
        
        ByteVector xorKey(newKeyData.size(), 0x00U);

        for (std::size_t i = 0; i < newKeyData.size(); ++i)
            xorKey[i] = static_cast<std::uint8_t>(newKeyData[i] ^ oldKey[i]);

        keyData.reserve(newKeyData.size() + 1U + 4U);
        keyData.insert(keyData.end(), xorKey.begin(), xorKey.end());
        keyData.push_back(newKey->getKeyVersion());
        
        // CRC32 is calculated over NewKey, not over XOR(NewKey, OldKey) or KeyVer
        // The CRC is appended least-significant byte first
        const std::uint32_t crc = desfire_crc32(newKeyData.data(), newKeyData.size());

        // CRC32 byte order used by the DESFire implementation : LSB first
        keyData.push_back(static_cast<std::uint8_t>(crc & 0xFFU));
        keyData.push_back(static_cast<std::uint8_t>((crc >> 8U) & 0xFFU));
        keyData.push_back(static_cast<std::uint8_t>((crc >> 16U) & 0xFFU));
        keyData.push_back(static_cast<std::uint8_t>((crc >> 24U) & 0xFFU));
    }

    /*
     * Expected DUOX KeyData lengths :
     *
     *   AES-128 / authenticated key = 17
     *   AES-128 / other key         = 21
     *   AES-256 / authenticated key = 33
     *   AES-256 / other key         = 37
     */
    EXCEPTION_ASSERT_WITH_LOG(keyData.size() == 17 || keyData.size() == 21 || keyData.size() == 33 || keyData.size() == 37,
        LibLogicalAccessException, "DUOX key change generated an invalid KeyData length.");

    return keyData;
}

} // namespace logicalaccess