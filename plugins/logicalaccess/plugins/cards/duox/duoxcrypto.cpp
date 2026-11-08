#include <logicalaccess/plugins/cards/duox/duoxcrypto.hpp>

#include <cstdint>
#include <memory>
#include <utility>

#include <logicalaccess/myexception.hpp>
#include <logicalaccess/plugins/cards/desfire/desfirecommands.hpp>

namespace logicalaccess
{

ByteVector DUOXCrypto::rotateAuthenticationChallenge(const ByteVector &rnd)
{
    return DESFireEV2Crypto::rotateAuthenticationChallenge(rnd);
}

std::pair<ByteVector, ByteVector> DUOXCrypto::deriveSessionKeys(
    const ByteVector &key, const ByteVector &rndA, const ByteVector &rndB)
{
    return deriveEV2SessionKeys(key, rndA, rndB);
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