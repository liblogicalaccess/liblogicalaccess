#include <logicalaccess/plugins/cards/desfire/transactionsignaturecrypto.hpp>

#include <limits>
#include <memory>
#include <string>

#include <logicalaccess/myexception.hpp>
#include <logicalaccess/plugins/llacommon/logs.hpp>

#include <openssl/bn.h>
#include <openssl/ecdsa.h>
#include <openssl/err.h>
#include <openssl/evp.h>

namespace logicalaccess
{
namespace
{

using ECDSASignaturePtr = std::unique_ptr<ECDSA_SIG, decltype(&ECDSA_SIG_free)>;
using BIGNUMPtr = std::unique_ptr<BIGNUM, decltype(&BN_free)>;
using EVP_MD_CTXPtr = std::unique_ptr<EVP_MD_CTX, decltype(&EVP_MD_CTX_free)>;

/**
 * \brief Converts a raw 64-byte ECDSA signature (r || s) to DER.
 */
ByteVector rawSignatureToDER(const DESFireTransactionSignatureCrypto::TransactionSignatureValue &raw)
{
    constexpr std::size_t COMPONENT_SIZE = 32U;

    BIGNUMPtr r(BN_bin2bn(raw.data(), static_cast<int>(COMPONENT_SIZE), nullptr), &BN_free);
    BIGNUMPtr s(BN_bin2bn(raw.data() + COMPONENT_SIZE, static_cast<int>(COMPONENT_SIZE), nullptr), &BN_free);

    EXCEPTION_ASSERT_WITH_LOG(r != nullptr && s != nullptr,
        LibLogicalAccessException, "TSV verification : failed to decode ECDSA signature components.");

    ECDSASignaturePtr signature(ECDSA_SIG_new(), &ECDSA_SIG_free);

    EXCEPTION_ASSERT_WITH_LOG(signature != nullptr,
        LibLogicalAccessException, "TSV verification : failed to allocate ECDSA signature.");

    // ECDSA_SIG_set0 takes ownership of r and s on success
    EXCEPTION_ASSERT_WITH_LOG(ECDSA_SIG_set0(signature.get(), r.get(), s.get()) == 1,
        LibLogicalAccessException, "TSV verification : failed to construct ECDSA signature.");

    (void)r.release();
    (void)s.release();

    const int derSize = i2d_ECDSA_SIG(signature.get(), nullptr);

    EXCEPTION_ASSERT_WITH_LOG(derSize > 0,
        LibLogicalAccessException, "TSV verification : failed to determine DER signature size.");

    ByteVector der(static_cast<std::size_t>(derSize));
    unsigned char *cursor = der.data();

    EXCEPTION_ASSERT_WITH_LOG(i2d_ECDSA_SIG(signature.get(), &cursor) == derSize,
        LibLogicalAccessException, "TSV verification : failed to encode DER signature.");

    return der;
}

} // namespace

bool DESFireTransactionSignatureCrypto::verifyTSV(const ByteVector &tmi,
                                                  const TransactionCounter &committedTMC,
                                                  const TransactionSignatureValue &tsv,
                                                  const EVP_PKEY *publicKey)
{
    EXCEPTION_ASSERT_WITH_LOG(publicKey != nullptr, LibLogicalAccessException,
                              "TSV verification : public key must not be null.");

    EXCEPTION_ASSERT_WITH_LOG(EVP_PKEY_base_id(publicKey) == EVP_PKEY_EC,
        LibLogicalAccessException, "TSV verification : the supplied key is not an EC public key.");

    EXCEPTION_ASSERT_WITH_LOG(tmi.size() <= (std::numeric_limits<std::size_t>::max)() - committedTMC.size(),
        LibLogicalAccessException, "TSV verification : message size overflow.");

    // TMI || the four byte committed (incremented) TMC
    ByteVector message;
    message.reserve(tmi.size() + committedTMC.size());
    message.insert(message.end(), tmi.begin(), tmi.end());
    message.insert(message.end(), committedTMC.begin(), committedTMC.end());

    const ByteVector derSignature = rawSignatureToDER(tsv);

    EVP_MD_CTXPtr context(EVP_MD_CTX_new(), &EVP_MD_CTX_free);

    EXCEPTION_ASSERT_WITH_LOG(context != nullptr,
        LibLogicalAccessException, "TSV verification : failed to allocate digest context.");

    ERR_clear_error();

    EXCEPTION_ASSERT_WITH_LOG(EVP_DigestVerifyInit(context.get(), nullptr, EVP_sha256(), nullptr,
                             const_cast<EVP_PKEY *>(publicKey)) == 1,
        LibLogicalAccessException, "TSV verification : failed to initialize ECDSA/SHA-256.");

    EXCEPTION_ASSERT_WITH_LOG(EVP_DigestVerifyUpdate(context.get(), message.data(), message.size()) == 1,
        LibLogicalAccessException, "TSV verification : failed to process transaction input.");

    const int verificationResult = EVP_DigestVerifyFinal(context.get(), derSignature.data(), derSignature.size());

    // OpenSSL returns 1 for a valid signature, 0 for an invalid signature and a negative value for an operational error
    if (verificationResult == 1)
        return true;

    if (verificationResult == 0)
    {
        ERR_clear_error();
        return false;
    }

    EXCEPTION_ASSERT_WITH_LOG(false,
        LibLogicalAccessException, "TSV verification : OpenSSL encountered a verification error.");

    return false;
}

} // namespace logicalaccess