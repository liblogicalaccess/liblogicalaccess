//
// Created by xaqq on 4/4/18.
//

#include <logicalaccess/plugins/llacommon/logs.hpp>
#include <logicalaccess/plugins/crypto/signature_helper.hpp>
#include <stdexcept>
#include <cassert>
#include <openssl/core.h>
#include <openssl/core_names.h>
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

bool SignatureHelper::verify_ecdsa_secp224r1(const std::vector<unsigned char> &data,
                                             const std::vector<unsigned char> &signature,
                                             const std::vector<unsigned char> &pubkey)
{
    bool ret = true;
    EVP_PKEY_CTX *ctx;
    EVP_PKEY *pkey = NULL;
    OSSL_PARAM_BLD *param_bld;
    OSSL_PARAM *params = NULL;
    param_bld = OSSL_PARAM_BLD_new();
    if (param_bld != NULL
        && OSSL_PARAM_BLD_push_utf8_string(param_bld, OSSL_PKEY_PARAM_GROUP_NAME, "secp224r1", 0)
        && OSSL_PARAM_BLD_push_octet_string(param_bld, OSSL_PKEY_PARAM_PUB_KEY, pubkey.data(), pubkey.size()))
    {
        params = OSSL_PARAM_BLD_to_param(param_bld);
    }
    ctx = EVP_PKEY_CTX_new_from_name(NULL, "EC", NULL);
    //EVP_PKEY_CTX_set_params(ctx, params);
    if (ctx == NULL || params == NULL)
    {
        LOG(LogLevel::ERRORS) << "Error when allocating EVP objects.";
        ret = false;
    }
    else if (EVP_PKEY_fromdata_init(ctx) <= 0)
    {
        LOG(LogLevel::ERRORS) << "Error when initializing EVP PKEY from data.";
        ret = false;
    }
    else if (EVP_PKEY_fromdata(ctx, &pkey, EVP_PKEY_PUBLIC_KEY, params) <= 0)
    {
        LOG(LogLevel::ERRORS) << "Error when initializing EVP PKEY public key.";
        ret = false;
    }

    EVP_PKEY_CTX_free(ctx);
    ctx = EVP_PKEY_CTX_new(pkey, NULL);
    if (ctx == NULL)
    {
        LOG(LogLevel::ERRORS) << "Error when allocating EVP objects (2).";
        ret = false;
    }
    else if (EVP_PKEY_verify_init(ctx) <= 0)
    {
        LOG(LogLevel::ERRORS) << "Error when initializing EVP PKEY for verify operation.";
        ret = false;
    }
    else
    {
        ByteVector sig;
        if (signature.size() == 56) // raw format
        {
            // Convert to asn1 format
            sig.push_back(0x30); // SEQUENCE tag
            sig.push_back(0x3c); // length
            sig.push_back(0x02); // INTEGER tag
            sig.push_back(0x1c); // length
            sig.insert(sig.end(), signature.begin(), signature.begin() + 28);
            sig.push_back(0x02); // INTEGER tag
            sig.push_back(0x1c); // length
            sig.insert(sig.end(), signature.end() - 28, signature.end());
        }
        else if (signature.size() == 64) // asn1 format
        {
            sig = signature;
        }
        else
        {
            LOG(LogLevel::ERRORS) << "Unexpected signature size {" << signature.size() << "}. "
                                  << "It should either be 56 (raw format) or 64 (asn1 format).";
            ret = false;
        }


        if (sig.size() > 0)
        {
            int rcode = EVP_PKEY_verify(ctx, sig.data(), sig.size(), data.data(), data.size());
            ret = (rcode == 1);
            if (rcode < 0)
            {
                LOG(LogLevel::ERRORS) << "Error when running EVP verify operation {" << rcode << "}.";
            }
        }
    }

    EVP_PKEY_free(pkey);
    EVP_PKEY_CTX_free(ctx);
    OSSL_PARAM_free(params);
    OSSL_PARAM_BLD_free(param_bld);
    return ret;
}
}
