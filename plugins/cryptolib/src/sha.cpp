/**
 * \file sha.cpp
 * \author Julien K. <julien-dev@islog.com>
 * \brief SHA hash functions.
 */

#include "logicalaccess/crypto/sha.hpp"

#include <openssl/evp.h>

namespace logicalaccess
{
    namespace openssl
    {
        std::vector<unsigned char> SHA256Hash(const std::vector<unsigned char>& buffer)
        {
            unsigned int len = 32;
            std::vector<unsigned char> r(len);

            EVP_MD_CTX mdctx;

            EVP_MD_CTX_init(&mdctx);

            EVP_DigestInit_ex(&mdctx, EVP_sha256(), NULL);
            EVP_DigestUpdate(&mdctx, &buffer[0], buffer.size());
            EVP_DigestFinal_ex(&mdctx, reinterpret_cast<unsigned char*>(&r[0]), &len);
            r.resize(len);

            EVP_MD_CTX_cleanup(&mdctx);

            return r;
        }

        std::vector<unsigned char> SHA256Hash(const std::string& str)
        {
            unsigned int len = 32;
            std::vector<unsigned char> r(len);

            EVP_MD_CTX mdctx;

            EVP_MD_CTX_init(&mdctx);

            EVP_DigestInit_ex(&mdctx, EVP_sha256(), NULL);
            EVP_DigestUpdate(&mdctx, str.c_str(), str.length());
            EVP_DigestFinal_ex(&mdctx, reinterpret_cast<unsigned char*>(&r[0]), &len);
            r.resize(len);

            EVP_MD_CTX_cleanup(&mdctx);

            return r;
        }
    }
}