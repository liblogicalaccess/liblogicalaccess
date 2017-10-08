/**
 * \file aes.cpp
 * \author Julien K. <julien-dev@islog.com>
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Initialization vector base class.
 */

#include "logicalaccess/logs.hpp"
#include "logicalaccess/myexception.hpp"
#include "logicalaccess/crypto/openssl.hpp"
#include "logicalaccess/crypto/initialization_vector.hpp"

#include <cstring>
#include <chrono>
#include <openssl/rand.h>
#include <openssl/evp.h>

namespace logicalaccess
{
    namespace openssl
    {
        InitializationVector::InitializationVector(size_t size, bool random) :
            d_data(size)
        {
            OpenSSLInitializer::GetInstance();

            if (random)
            {
                randomize();
            }
            else
            {
                zero();
            }
        }

        InitializationVector::InitializationVector(const ByteVector& _data) :
            d_data(_data)
        {
        }

        void InitializationVector::zero()
        {
            size_t size = d_data.size();
            d_data.clear();
            d_data.resize(size, 0x00);
        }

        void InitializationVector::randomize()
        {
            if (RAND_bytes(&d_data[0], static_cast<int>(d_data.size())) != 1)
            {
                THROW_EXCEPTION_WITH_LOG(logicalaccess::LibLogicalAccessException, "Cannot retrieve cryptographically strong bytes");
            }
        }
    }
}