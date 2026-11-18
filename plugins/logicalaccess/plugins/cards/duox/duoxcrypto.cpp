#include <logicalaccess/plugins/cards/duox/duoxcrypto.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <utility>

#include <openssl/bn.h>
#include <openssl/core_names.h>
#include <openssl/ecdsa.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/params.h>
#include <openssl/param_build.h>

#include <logicalaccess/myexception.hpp>
#include <logicalaccess/plugins/cards/desfire/desfirecommands.hpp>
#include <logicalaccess/plugins/crypto/aes_helper.hpp>
#include <logicalaccess/plugins/crypto/aes_symmetric_key.hpp>
#include <logicalaccess/plugins/crypto/cmac.hpp>
#include <logicalaccess/plugins/crypto/secure_memory.hpp>

// TODO : Replace throw by EXCEPTION_ASSERT_WITH_LOG. But this will be part of a larger refactor.
// Don't touch it for now.

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

constexpr std::size_t KDF_CONTEXT_SIZE = 26U;
constexpr std::size_t KDF_VECTOR_SIZE  = 32U;

// ========================================
// EV CHARGING
// ========================================

using EVP_PKEY_ptr = std::unique_ptr<EVP_PKEY, decltype(&EVP_PKEY_free)>;
using EVP_PKEY_CTX_ptr = std::unique_ptr<EVP_PKEY_CTX, decltype(&EVP_PKEY_CTX_free)>;
using EVP_MD_CTX_ptr = std::unique_ptr<EVP_MD_CTX, decltype(&EVP_MD_CTX_free)>;
using ECDSA_SIG_ptr = std::unique_ptr<ECDSA_SIG, decltype(&ECDSA_SIG_free)>;
using BIGNUM_ptr = std::unique_ptr<BIGNUM, decltype(&BN_free)>;
using OSSL_PARAM_BLD_ptr = std::unique_ptr<OSSL_PARAM_BLD, decltype(&OSSL_PARAM_BLD_free)>;
using OSSL_PARAM_ptr = std::unique_ptr<OSSL_PARAM, decltype(&OSSL_PARAM_free)>;

constexpr char BRAINPOOL_P256R1[] = "brainpoolP256r1";

EVP_PKEY_ptr createBrainpoolPublicKey(const ByteVector &publicKey)
{
    if (publicKey.size() != duox_ev_charging::crypto::PUBLIC_KEY_SIZE || publicKey.front() != 0x04U)
        throw LibLogicalAccessException("Invalid BrainpoolP256r1 public key encoding.");

    EVP_PKEY_CTX_ptr context(EVP_PKEY_CTX_new_from_name(nullptr, "EC", nullptr), &EVP_PKEY_CTX_free);

    if (!context)
        throw LibLogicalAccessException("Unable to create OpenSSL EC provider context.");

    OSSL_PARAM_BLD_ptr builder(OSSL_PARAM_BLD_new(), &OSSL_PARAM_BLD_free);

    if (!builder)
        throw LibLogicalAccessException("Unable to allocate OpenSSL parameter builder.");

    if (OSSL_PARAM_BLD_push_utf8_string(builder.get(), OSSL_PKEY_PARAM_GROUP_NAME, BRAINPOOL_P256R1, 0U) != 1)
        throw LibLogicalAccessException("Unable to configure BrainpoolP256r1 curve.");

    if (OSSL_PARAM_BLD_push_octet_string(builder.get(), OSSL_PKEY_PARAM_PUB_KEY, publicKey.data(), publicKey.size()) != 1)
        throw LibLogicalAccessException("Unable to configure EC public key.");

    OSSL_PARAM_ptr parameters(OSSL_PARAM_BLD_to_param(builder.get()), &OSSL_PARAM_free);

    if (!parameters)
        throw LibLogicalAccessException("Unable to construct OpenSSL EC parameters.");

    if (EVP_PKEY_fromdata_init(context.get()) != 1)
        throw LibLogicalAccessException("Unable to initialize EVP_PKEY_fromdata.");

    EVP_PKEY *rawKey = nullptr;

    if (EVP_PKEY_fromdata(context.get(), &rawKey, EVP_PKEY_PUBLIC_KEY, parameters.get()) != 1)
    {
        EVP_PKEY_free(rawKey);
        throw LibLogicalAccessException("Unable to import BrainpoolP256r1 public key.");
    }

    EVP_PKEY_ptr key(rawKey, &EVP_PKEY_free);

    // Verify the imported key using the provider
    EVP_PKEY_CTX_ptr validationContext(EVP_PKEY_CTX_new_from_pkey(nullptr, key.get(), nullptr), &EVP_PKEY_CTX_free);

    if (!validationContext)
        throw LibLogicalAccessException("Unable to create EC public key validation context.");

    if (EVP_PKEY_public_check(validationContext.get()) != 1)
        throw LibLogicalAccessException("Invalid BrainpoolP256r1 public key.");

    return key;
}

ByteVector rawECDSAToDER(const ByteVector &signature)
{
    if (signature.size() != duox_ev_charging::crypto::SIGNATURE_SIZE)
        throw LibLogicalAccessException("ECDSA signature must contain exactly 64 bytes.");

    constexpr std::size_t COORDINATE_SIZE = duox_ev_charging::crypto::COORDINATE_SIZE;

    BIGNUM_ptr r(BN_bin2bn(signature.data(), static_cast<int>(COORDINATE_SIZE), nullptr), &BN_free);
    BIGNUM_ptr s(BN_bin2bn(signature.data() + COORDINATE_SIZE, static_cast<int>(COORDINATE_SIZE), nullptr), &BN_free);

    if (!r || !s)
        throw LibLogicalAccessException("Unable to decode ECDSA signature integers.");

    ECDSA_SIG_ptr sig(ECDSA_SIG_new(), &ECDSA_SIG_free);

    if (!sig)
        throw LibLogicalAccessException("Unable to allocate ECDSA signature.");

    if (ECDSA_SIG_set0(sig.get(), r.get(), s.get()) != 1)
        throw LibLogicalAccessException("Unable to construct ECDSA signature.");

    // Ownership transferred to ECDSA_SIG
    r.release();
    s.release();

    const BIGNUM *sigR = nullptr;
    const BIGNUM *sigS = nullptr;

    ECDSA_SIG_get0(sig.get(), &sigR, &sigS);

    // Reject zero and negative integers
    if (BN_is_zero(sigR) || BN_is_negative(sigR) || BN_is_zero(sigS) || BN_is_negative(sigS))
        throw LibLogicalAccessException("ECDSA signature contains invalid integers.");

    const int derLength = i2d_ECDSA_SIG(sig.get(), nullptr);

    if (derLength <= 0)
        throw LibLogicalAccessException("Unable to calculate ECDSA DER length.");

    ByteVector der(static_cast<std::size_t>(derLength));

    unsigned char *cursor = der.data();

    if (i2d_ECDSA_SIG(sig.get(), &cursor) != derLength)
        throw LibLogicalAccessException("ECDSA DER encoding failed.");

    return der;
}

bool verifyBrainpoolECDSA(const ByteVector &publicKey, const ByteVector &message, const ByteVector &signature)
{
    if (message.empty())
        throw LibLogicalAccessException("ECDSA verification message cannot be empty.");

    EVP_PKEY_ptr key = createBrainpoolPublicKey(publicKey);

    const ByteVector derSignature = rawECDSAToDER(signature);

    EVP_MD_CTX_ptr context(EVP_MD_CTX_new(), &EVP_MD_CTX_free);

    if (!context)
        throw LibLogicalAccessException("Unable to allocate EVP_MD_CTX.");

    if (EVP_DigestVerifyInit(context.get(), nullptr, EVP_sha256(), nullptr, key.get()) != 1)
        throw LibLogicalAccessException("Unable to initialize ECDSA/SHA-256 verification.");

    if (EVP_DigestVerifyUpdate(context.get(), message.data(), message.size()) != 1)
        throw LibLogicalAccessException("ECDSA/SHA-256 digest update failed.");

    const int result = EVP_DigestVerifyFinal(context.get(), derSignature.data(), derSignature.size());

    if (result == 1)
        return true;

    if (result == 0)
        return false;

    throw LibLogicalAccessException("OpenSSL ECDSA verification operation failed.");
}

std::uint8_t decodeBCD(std::uint8_t value)
{
    const std::uint8_t high = static_cast<std::uint8_t>(value >> 4U);
    const std::uint8_t low = static_cast<std::uint8_t>(value & 0x0FU);

    if (high > 9U || low > 9U)
        throw LibLogicalAccessException("Invalid BCD digit in EV Charging certificate date.");

    return static_cast<std::uint8_t>(high * 10U + low);
}

bool isLeapYear(std::uint16_t year)
{
    return (year % 400U == 0U) || ((year % 4U == 0U) && (year % 100U != 0U));
}

EVChargingDate decodeEVChargingDate(const ByteVector &encoded)
{
    if (encoded.size() != duox_ev_charging::certificate::DATE_SIZE)
        throw LibLogicalAccessException("EV Charging certificate date must contain 4 bytes.");

    EVChargingDate result;

    result.year = static_cast<std::uint16_t>(decodeBCD(encoded[0]) * 100U + decodeBCD(encoded[1]));
    result.month = decodeBCD(encoded[2]);
    result.day   = decodeBCD(encoded[3]);

    if (result.year == 0U || result.month < 1U || result.month > 12U)
        throw LibLogicalAccessException("Invalid EV Charging certificate date.");

    static constexpr std::uint8_t DAYS_PER_MONTH[] = {31U, 28U, 31U, 30U, 31U, 30U,
                                                      31U, 31U, 30U, 31U, 30U, 31U};

    std::uint8_t maxDay = DAYS_PER_MONTH[result.month - 1U];

    if (result.month == 2U && isLeapYear(result.year))
        maxDay = 29U;

    if (result.day < 1U || result.day > maxDay)
        throw LibLogicalAccessException("Invalid day in EV Charging certificate date.");

    return result;
}

bool isValidBCD(const ByteVector &data)
{
    return std::all_of(data.begin(), data.end(), [](std::uint8_t value)
                       { return (value >> 4U) <= 9U && (value & 0x0FU) <= 9U; });
}

struct BERTLVObject
{
    std::uint32_t tag = 0U;

    std::size_t offset        = 0U;
    std::size_t headerLength  = 0U;
    std::size_t valueOffset   = 0U;
    std::size_t valueLength   = 0U;
    std::size_t encodedLength = 0U;
};

// TODO : this class will be part of a larger refactor (with tlv.cpp). Don't touch it for now.
class BERTLVReader
{
  public:
    explicit BERTLVReader(const ByteVector &buffer)
        : buffer_(buffer)
    {
    }

    BERTLVObject read(std::size_t &offset) const
    {
        const std::size_t start = offset;

        requireAvailable(offset, 1U);

        BERTLVObject object;
        object.offset = start;

        // ----------------------------------------
        // Tag decoding
        // ----------------------------------------

        std::uint8_t firstTagByte = buffer_[offset++];

        object.tag = firstTagByte;

        if ((firstTagByte & 0x1FU) == 0x1FU)
        {
            bool terminated = false;

            for (unsigned int i = 0U; i < 3U; ++i)
            {
                requireAvailable(offset, 1U);

                const std::uint8_t next = buffer_[offset++];

                if (object.tag > ((std::numeric_limits<std::uint32_t>::max)() >> 8U))
                    throw LibLogicalAccessException("BER-TLV tag overflow.");

                object.tag = (object.tag << 8U) | next;

                if ((next & 0x80U) == 0U)
                {
                    terminated = true;
                    break;
                }
            }

            if (!terminated)
                throw LibLogicalAccessException("Invalid or excessively long BER-TLV tag.");
        }

        // ----------------------------------------
        // Length decoding
        // ----------------------------------------

        requireAvailable(offset, 1U);

        const std::uint8_t firstLengthByte = buffer_[offset++];

        std::size_t length = 0U;

        if ((firstLengthByte & 0x80U) == 0U)
        {
            length = firstLengthByte;
        }
        else
        {
            const std::uint8_t lengthBytes = static_cast<std::uint8_t>(firstLengthByte & 0x7FU);

            // Indefinite lengths are not accepted
            if (lengthBytes == 0U)
                throw LibLogicalAccessException("Indefinite BER-TLV lengths are not supported.");

            // The EV Charging certificate is small
            if (lengthBytes > sizeof(std::size_t))
                throw LibLogicalAccessException("BER-TLV length exceeds supported size.");

            requireAvailable(offset, lengthBytes);

            for (std::uint8_t i = 0U; i < lengthBytes; ++i)
            {
                const std::uint8_t byte = buffer_[offset++];

                if (length > ((std::numeric_limits<std::size_t>::max)() >> 8U))
                    throw LibLogicalAccessException("BER-TLV length overflow.");

                length = (length << 8U) | byte;
            }

            // Reject non-minimal long-form lengths
            if (length < 128U)
                throw LibLogicalAccessException("Non-canonical BER-TLV length encoding.");

            if (lengthBytes > 1U)
            {
                const std::size_t minimum = static_cast<std::size_t>(1U) << (8U * (lengthBytes - 1U));

                if (length < minimum)
                    throw LibLogicalAccessException("Non-minimal BER-TLV length encoding.");
            }
        }

        object.headerLength = offset - start;
        object.valueOffset  = offset;
        object.valueLength  = length;

        requireAvailable(offset, length);

        object.encodedLength = object.headerLength + object.valueLength;

        offset += length;

        return object;
    }

    ByteVector extract(const BERTLVObject &object) const
    {
        return ByteVector(buffer_.begin() + object.valueOffset, buffer_.begin() + object.valueOffset + object.valueLength);
    }

    void requireAvailable(std::size_t offset, std::size_t length) const
    {
        if (offset > buffer_.size() || length > buffer_.size() - offset)
            throw LibLogicalAccessException("Truncated BER-TLV object.");
    }

  private:
    const ByteVector &buffer_;
};

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
        LibLogicalAccessException, "Internal DUOX EV2 error : invalid KDF context size.");

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
            LibLogicalAccessException, "DUOX EV2 KDF CMAC returned an invalid 16 byte block.");

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

void DUOXCrypto::VDE_ValidateChallenge(const ByteVector &challenge)
{
    if (challenge.size() != duox_ev_charging::crypto::CHALLENGE_SIZE)
        throw LibLogicalAccessException("EV Charging challenge must contain exactly 32 bytes.");
}

void DUOXCrypto::VDE_ValidateSignature(const ByteVector &signature)
{
    if (signature.size() != duox_ev_charging::crypto::SIGNATURE_SIZE)
        throw LibLogicalAccessException("EV Charging ECDSA signature must contain exactly 64 bytes.");
}

void DUOXCrypto::VDE_ValidatePublicKey(const ByteVector &publicKey)
{
    (void)createBrainpoolPublicKey(publicKey);
}

bool DUOXCrypto::VDE_ECDSAVerify(const ByteVector &publicKey, const ByteVector &challenge, const ByteVector &signature)
{
    VDE_ValidatePublicKey(publicKey);
    VDE_ValidateChallenge(challenge);
    VDE_ValidateSignature(signature);

    return verifyBrainpoolECDSA(publicKey, challenge, signature);
}

EVChargingCertificate DUOXCrypto::VDE_ParseCertificate(const ByteVector &certificate)
{
    using namespace duox_ev_charging::certificate;

    if (certificate.empty())
        throw LibLogicalAccessException("EV Charging certificate cannot be empty.");

    // ----------------------------------------
    // Outer certificate
    // ----------------------------------------

    BERTLVReader outerReader(certificate);

    std::size_t offset = 0U;

    const BERTLVObject outer = outerReader.read(offset);

    if (outer.tag != TAG_CERTIFICATE)
        throw LibLogicalAccessException("Invalid EV Charging certificate outer tag.");

    // The outer certificate must occupy the entire logical input.
    // Zero padding is permitted after the encoded object.
    for (std::size_t i = offset; i < certificate.size(); ++i)
    {
        if (certificate[i] != 0x00U)
            throw LibLogicalAccessException("Unexpected trailing data after EV Charging certificate.");
    }

    EVChargingCertificate result;

    result.rawData = certificate;

    // ----------------------------------------
    // Certificate fields
    // ----------------------------------------

    const std::size_t contentEnd = outer.valueOffset + outer.valueLength;
    std::size_t cursor = outer.valueOffset;

    auto readExpected = [&](std::uint32_t expectedTag)
    {
        if (cursor >= contentEnd)
            throw LibLogicalAccessException("Missing mandatory EV Charging certificate field.");

        const BERTLVObject object = outerReader.read(cursor);

        if (object.tag != expectedTag)
            throw LibLogicalAccessException("Unexpected EV Charging certificate TLV tag.");

        return object;
    };

    // The first signed object starts here.
    const std::size_t signedDataStart = cursor;

    // 93 - UID
    const auto uidObject = readExpected(TAG_UID);

    result.uid = outerReader.extract(uidObject);

    if (result.uid.size() != 7U && result.uid.size() != 10U)
        throw LibLogicalAccessException("EV Charging certificate UID must contain 7 or 10 bytes.");

    // 42 - CA-ID
    const auto caObject = readExpected(TAG_CA_ID);

    result.caId = outerReader.extract(caObject);

    if (result.caId.size() != 7U)
        throw LibLogicalAccessException("EV Charging certificate CA-ID must contain 7 bytes.");

    // 5720 - Subject Identifier
    const auto subjectObject = readExpected(TAG_SUBJECT_IDENTIFIER);

    result.subjectIdentifier = outerReader.extract(subjectObject);

    if (result.subjectIdentifier.size() != result.uid.size())
        throw LibLogicalAccessException("EV Charging subject identifier length does not match UID.");

    if (result.subjectIdentifier != result.uid)
        throw LibLogicalAccessException("EV Charging subject identifier does not match certificate UID.");

    // 95 - Key Usage
    const auto keyUsageObject = readExpected(TAG_KEY_USAGE);

    result.keyUsage = outerReader.extract(keyUsageObject);

    const ByteVector expectedKeyUsage = {0x02U, 0x00U};

    if (result.keyUsage != expectedKeyUsage)
        throw LibLogicalAccessException("Unsupported EV Charging certificate key usage.");

    // 5F25 - Effective Date
    const auto effectiveDateObject = readExpected(TAG_EFFECTIVE_DATE);

    result.effectiveDate = outerReader.extract(effectiveDateObject);

    // 5F24 - Expiration Date
    const auto expirationDateObject = readExpected(TAG_EXPIRATION_DATE);

    result.expirationDate = outerReader.extract(expirationDateObject);

    // Validate both dates immediately
    (void)decodeEVChargingDate(result.effectiveDate);
    (void)decodeEVChargingDate(result.expirationDate);

    // 45 - CA Security Domain Image Number
    const auto imageObject = readExpected(TAG_CA_SD_IMAGE_NUMBER);

    if (imageObject.valueLength != 1U)
        throw LibLogicalAccessException("Invalid CA Security Domain Image Number length.");

    result.caSecurityDomainImageNumber = certificate[imageObject.valueOffset];

    if (result.caSecurityDomainImageNumber != 0x00U)
        throw LibLogicalAccessException("Unsupported CA Security Domain Image Number.");

    // ----------------------------------------
    // 7F49 - Public Key Data Object
    // ----------------------------------------

    const auto publicKeyObject = readExpected(TAG_PUBLIC_KEY_DATA);

    result.publicKeyData = outerReader.extract(publicKeyObject);

    if (publicKeyObject.valueLength != 70U)
        throw LibLogicalAccessException("EV Charging public key data object must contain 70 bytes.");

    BERTLVReader keyReader(result.publicKeyData);

    std::size_t keyOffset = 0U;

    // B0 - Public key
    const auto publicKeyTLV = keyReader.read(keyOffset);

    if (publicKeyTLV.tag != TAG_PUBLIC_KEY || publicKeyTLV.valueLength != 65U)
        throw LibLogicalAccessException("Invalid EV Charging public key object.");

    result.publicKey = keyReader.extract(publicKeyTLV);

    // F0 - Key Parameter Reference
    const auto parameterTLV = keyReader.read(keyOffset);

    if (parameterTLV.tag != TAG_KEY_PARAMETER_REFERENCE || parameterTLV.valueLength != 1U)
        throw LibLogicalAccessException("Invalid EV Charging key parameter reference.");

    const std::uint8_t parameterReference = result.publicKeyData[parameterTLV.valueOffset];

    switch (parameterReference)
    {
    case static_cast<std::uint8_t>(duox_ev_charging::EVChargingKeyParameterReference::BrainpoolP256r1):

        result.keyParameterReference = duox_ev_charging::EVChargingKeyParameterReference::BrainpoolP256r1;

        break;

    default:
        throw LibLogicalAccessException("Unsupported EV Charging elliptic-curve parameter reference.");
    }

    if (keyOffset != result.publicKeyData.size())
        throw LibLogicalAccessException("Unexpected data inside EV Charging public key object.");

    // Validate actual curve membership
    DUOXCrypto::VDE_ValidatePublicKey(result.publicKey);

    // The signature covers bytes 93 through the complete 7F49
    const std::size_t signedDataEnd = cursor;

    result.signedData.assign(certificate.begin() + signedDataStart, certificate.begin() + signedDataEnd);

    // ----------------------------------------
    // 5F37 - Certificate Signature
    // ----------------------------------------

    const auto signatureObject = readExpected(TAG_SIGNATURE);

    result.signature = outerReader.extract(signatureObject);

    DUOXCrypto::VDE_ValidateSignature(result.signature);

    if (cursor != contentEnd)
        throw LibLogicalAccessException("Unexpected additional objects in EV Charging certificate.");

    // Parsing never establishes authenticity
    result.signatureVerified = false;

    VDE_ValidateCertificate(result);

    return result;
}

bool DUOXCrypto::VDE_VerifyCertificate(const EVChargingCertificate &certificate, const ByteVector &caPublicKey)
{
    VDE_ValidatePublicKey(caPublicKey);
    VDE_ValidateSignature(certificate.signature);

    if (certificate.signedData.empty())
        throw LibLogicalAccessException("EV Charging certificate has no signed data.");

    if (certificate.publicKey.empty())
        throw LibLogicalAccessException("EV Charging certificate has no public key.");

    // Validate the card public key independently
    VDE_ValidatePublicKey(certificate.publicKey);

    return verifyBrainpoolECDSA(caPublicKey, certificate.signedData, certificate.signature);
}

bool DUOXCrypto::VDE_VerifyCertificate(const ByteVector &certificate, const ByteVector &caPublicKey)
{
    const EVChargingCertificate parsed = VDE_ParseCertificate(certificate);

    return VDE_VerifyCertificate(parsed, caPublicKey);
}

bool DUOXCrypto::VDE_VerifyCertificateUID(const EVChargingCertificate &certificate, const ByteVector &cardUID)
{
    if (cardUID.size() != 7U && cardUID.size() != 10U)
        throw LibLogicalAccessException("EV Charging card UID must contain 7 or 10 bytes.");

    if (certificate.uid.size() != cardUID.size())
        return false;

    return std::equal(certificate.uid.begin(), certificate.uid.end(), cardUID.begin());
}

bool DUOXCrypto::VDE_IsCertificateValidAt(const EVChargingCertificate &certificate, const EVChargingDate &currentDate)
{
    const EVChargingDate effectiveDate = decodeEVChargingDate(certificate.effectiveDate);
    const EVChargingDate expirationDate = decodeEVChargingDate(certificate.expirationDate);

    if (expirationDate < effectiveDate)
        throw LibLogicalAccessException("EV Charging certificate expiration date precedes effective date.");

    return !(currentDate < effectiveDate) && !(expirationDate < currentDate);
}

void DUOXCrypto::VDE_ValidateCertificate(const EVChargingCertificate &certificate)
{
    if (certificate.uid.size() != 7U && certificate.uid.size() != 10U)
        throw LibLogicalAccessException("Invalid EV Charging certificate UID length.");

    if (certificate.caId.size() != duox_ev_charging::certificate::CA_ID_SIZE)
        throw LibLogicalAccessException("EV Charging certificate CA identifier must contain exactly 7 bytes.");

    if (!isValidBCD(certificate.caId))
        throw LibLogicalAccessException("EV Charging certificate CA identifier contains invalid BCD digits.");

    if (certificate.subjectIdentifier.empty())
        throw LibLogicalAccessException("EV Charging certificate subject identifier is empty.");

    if (certificate.keyUsage.empty())
        throw LibLogicalAccessException("EV Charging certificate key usage is empty.");

    if (certificate.effectiveDate.size() != duox_ev_charging::certificate::DATE_SIZE ||
        certificate.expirationDate.size() != duox_ev_charging::certificate::DATE_SIZE)
        throw LibLogicalAccessException("Invalid EV Charging certificate date encoding.");

    if (certificate.publicKeyData.empty())
        throw LibLogicalAccessException("EV Charging certificate public key data is empty.");

    VDE_ValidatePublicKey(certificate.publicKey);
    VDE_ValidateSignature(certificate.signature);

    if (certificate.signedData.empty())
        throw LibLogicalAccessException("EV Charging certificate signed data is empty.");

    if (certificate.caSecurityDomainImageNumber != 0x00U)
        throw LibLogicalAccessException("Unsupported EV Charging certificate CA Security Domain Image Number.");
}

ByteVector DUOXCrypto::calculateSHA256(const ByteVector &data)
{
    ByteVector digest(EVP_MAX_MD_SIZE);

    unsigned int digestLength = 0U;

    if (EVP_Digest(data.data(), data.size(), digest.data(), &digestLength, EVP_sha256(), nullptr) != 1)
        throw LibLogicalAccessException("SHA-256 calculation failed.");

    if (digestLength != duox_ev_charging::crypto::HASH_SIZE)
        throw LibLogicalAccessException("Unexpected SHA-256 digest length.");

    digest.resize(digestLength);

    return digest;
}

} // namespace logicalaccess