//
// Created by xaqq on 4/4/18.
//

#include <logicalaccess/plugins/crypto/signature_helper.hpp>
#include <openssl/evp.h>
#include <stdexcept>
#include <openssl/pem.h>
#include <cassert>

namespace logicalaccess
{

static void fail(const std::string &why)
{
    throw std::runtime_error("Signature verification error: " + why);
}

namespace
{
// Verification helper with RAII construct to not leak memory on error.
struct VerificationHelper
{
    explicit VerificationHelper(const std::string &pem_public_key)
        : bio(nullptr)
        , pkey(nullptr)
    {

#if OPENSSL_VERSION_NUMBER < 0x10100000L
        EVP_MD_CTX_init(&ctx);
#else
        ctx = EVP_MD_CTX_new();
        if (ctx == nullptr)
            throw std::runtime_error("EVP_MD_CTX_new() failed");
#endif

        // We cast away constness for older openssl version.
        bio = BIO_new_mem_buf(const_cast<char *>(pem_public_key.c_str()),
                              pem_public_key.size());
        if (bio == nullptr)
            fail("Cannot wrap public key in BIO object");

        pkey = PEM_read_bio_PUBKEY(bio, NULL, NULL, NULL);
        if (pkey == nullptr)
            fail("Cannot load public key");
    }

    EVP_MD_CTX *get_ctx()
    {
#if OPENSSL_VERSION_NUMBER < 0x10100000L
        return &ctx;
#else
        return ctx;
#endif
    }

    ~VerificationHelper()
    {
        EVP_PKEY_free(pkey);
        BIO_free_all(bio);
#if OPENSSL_VERSION_NUMBER < 0x10100000L
        EVP_MD_CTX_cleanup(&ctx);
#else
        EVP_MD_CTX_free(ctx);
#endif
    }
#if OPENSSL_VERSION_NUMBER < 0x10100000L
    EVP_MD_CTX ctx;
#else
    EVP_MD_CTX *ctx;
#endif
    BIO *bio;
    EVP_PKEY *pkey;
};
}

bool SignatureHelper::verify_sha512(const std::string &data, const std::string &signature,
                                    const std::string &pem_pubkey)
{
    VerificationHelper helper(pem_pubkey);

    if (1 !=
        EVP_DigestVerifyInit(helper.get_ctx(), NULL, EVP_sha512(), NULL, helper.pkey))
    {
        throw std::runtime_error("EVP_DigestVerifyInit");
    }

    if (1 != EVP_DigestVerifyUpdate(helper.get_ctx(), data.c_str(), data.size()))
    {
        throw std::runtime_error("EVP_DigestVerifyUpdate");
    }

    return 1 == EVP_DigestVerifyFinal(helper.get_ctx(),
                                      reinterpret_cast<unsigned char *>(
                                          const_cast<char *>(signature.c_str())),
                                      signature.size());
}
}
