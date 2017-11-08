/**
 * \file symmetric_key.cpp
 * \author Julien K. <julien-dev@islog.com>
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Symmetric key class.
 */

#include <logicalaccess/logs.hpp>
#include <logicalaccess/myexception.hpp>
#include <logicalaccess/plugins/crypto/openssl.hpp>
#include <logicalaccess/plugins/crypto/symmetric_key.hpp>

#include <cstring>
#include <chrono>
#include <stdlib.h>
#include <openssl/rand.h>

namespace logicalaccess
{
namespace openssl
{
SymmetricKey::SymmetricKey(size_t size)
    : d_data(size)
{
    OpenSSLInitializer::GetInstance();
    randomize();
}

SymmetricKey::SymmetricKey(const ByteVector &_data)
    : d_data(_data)
{
}

void SymmetricKey::randomize()
{
    if (RAND_bytes(&d_data[0], static_cast<int>(d_data.size())) != 1)
    {
        THROW_EXCEPTION_WITH_LOG(logicalaccess::LibLogicalAccessException,
                                 "Cannot retrieve cryptographically strong bytes");
    }
}
}
}