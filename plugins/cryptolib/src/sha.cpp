/**
 * \file sha.cpp
 * \author Julien K. <julien-dev@islog.com>
 * \brief SHA hash functions.
 */

#include "logicalaccess/crypto/sha.hpp"

#include <openssl/evp.h>
#include <cassert>

namespace logicalaccess
{
    namespace openssl
    {
        ByteVector SHA256Hash(const ByteVector& buffer)
        {
            unsigned int len = 32;
            ByteVector r(len);

            EVP_MD_CTX *mdctx_ptr = nullptr;
#if OPENSSL_VERSION_NUMBER < 0x10100000L
            EVP_MD_CTX mdctx;
            EVP_MD_CTX_init(&mdctx);
            mdctx_ptr = &mdctx;
#else
            EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
            assert(mdctx && "Cannot allocate SSL MD CTX");
            mdctx_ptr = mdctx;
#endif

            EVP_DigestInit_ex(mdctx_ptr, EVP_sha256(), nullptr);
            EVP_DigestUpdate(mdctx_ptr, &buffer[0], buffer.size());
            EVP_DigestFinal_ex(mdctx_ptr, reinterpret_cast<unsigned char*>(&r[0]), &len);
            r.resize(len);

#if OPENSSL_VERSION_NUMBER < 0x10100000L
            EVP_MD_CTX_cleanup(&mdctx);
#else
            EVP_MD_CTX_destroy(mdctx);
#endif

            return r;
        }

        ByteVector SHA256Hash(const std::string& str)
        {
            unsigned int len = 32;
            ByteVector r(len);

            EVP_MD_CTX *mdctx_ptr = nullptr;
#if OPENSSL_VERSION_NUMBER < 0x10100000L
            EVP_MD_CTX mdctx;
            EVP_MD_CTX_init(&mdctx);
            mdctx_ptr = &mdctx;
#else
            EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
            assert(mdctx && "Cannot allocate SSL MD CTX");
            mdctx_ptr = mdctx;
#endif

            EVP_DigestInit_ex(mdctx_ptr, EVP_sha256(), nullptr);
            EVP_DigestUpdate(mdctx_ptr, str.c_str(), str.length());
            EVP_DigestFinal_ex(mdctx_ptr, reinterpret_cast<unsigned char*>(&r[0]), &len);
            r.resize(len);

#if OPENSSL_VERSION_NUMBER < 0x10100000L
            EVP_MD_CTX_cleanup(&mdctx);
#else
            EVP_MD_CTX_destroy(mdctx);
#endif

            return r;
        }

    std::vector<uint8_t> SHA1Hash(const std::vector<uint8_t> &in)
    {
        unsigned int len = 20;
        ByteVector r(len);

        EVP_MD_CTX *mdctx_ptr = nullptr;
#if OPENSSL_VERSION_NUMBER < 0x10100000L
        EVP_MD_CTX mdctx;
        EVP_MD_CTX_init(&mdctx);
        mdctx_ptr = &mdctx;
#else
        EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
        assert(mdctx && "Cannot allocate SSL MD CTX");
        mdctx_ptr = mdctx;
#endif

        EVP_DigestInit_ex(mdctx_ptr, EVP_sha1(), nullptr);
        EVP_DigestUpdate(mdctx_ptr, &in[0], in.size());
        EVP_DigestFinal_ex(mdctx_ptr, reinterpret_cast<unsigned char*>(&r[0]), &len);
        r.resize(len);

#if OPENSSL_VERSION_NUMBER < 0x10100000L
        EVP_MD_CTX_cleanup(&mdctx);
#else
        EVP_MD_CTX_destroy(mdctx);
#endif

        return r;
    }
    }
}