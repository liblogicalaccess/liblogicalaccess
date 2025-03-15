//
// Created by xaqq on 4/4/18.
//

#include <logicalaccess/plugins/crypto/signature_helper.hpp>
#include <stdexcept>
#include <cassert>
#include <openssl/core.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/param_build.h>

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

bool SignatureHelper::verify_ecdsa_secp224r1(const std::vector<uint8_t> &data,
                                             const std::vector<uint8_t> &signature,
                                             const std::vector<uint8_t> &pubkey)
{
    bool ret;
    EVP_PKEY_CTX *ctx;
    //EVP_PKEY *pkey = NULL;
    OSSL_PARAM_BLD *param_bld;
    OSSL_PARAM *params = NULL;
    param_bld = OSSL_PARAM_BLD_new();
    if (param_bld != NULL
        && OSSL_PARAM_BLD_push_utf8_string(param_bld, "group", "secp224r1", 0)
        && OSSL_PARAM_BLD_push_octet_string(param_bld, "pub", pubkey.data(), pubkey.size()))
    {
        params = OSSL_PARAM_BLD_to_param(param_bld);
    }
    ctx = EVP_PKEY_CTX_new_from_name(NULL, "EC", NULL);
    if (ctx == NULL || params == NULL || EVP_PKEY_fromdata_init(ctx) <= 0 || EVP_PKEY_verify_init_ex(ctx, params) <= 0) //  || EVP_PKEY_fromdata(ctx, &pkey, EVP_PKEY_PUBLIC_KEY, params) <= 0
    {
        ret = false;
    }
    else
    {
        ret = (EVP_PKEY_verify(ctx, signature.data(), signature.size(), data.data(), data.size()) == 1);
    }
    //EVP_PKEY_free(pkey);
    EVP_PKEY_CTX_free(ctx);
    OSSL_PARAM_free(params);
    OSSL_PARAM_BLD_free(param_bld);
    return ret;
}
}
