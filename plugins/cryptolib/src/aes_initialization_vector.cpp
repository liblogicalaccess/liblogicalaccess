/**
 * \file aes_initialization_vector.cpp
 * \author Julien K. <julien-dev@islog.com>
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief AES initialization vector class.
 */

#include "logicalaccess/crypto/aes_initialization_vector.hpp"
#include <cassert>

namespace logicalaccess
{
    namespace openssl
    {
        AESInitializationVector AESInitializationVector::createNull()
        {
            return AESInitializationVector(false);
        }

        AESInitializationVector AESInitializationVector::createRandom()
        {
            return AESInitializationVector(true);
        }

        AESInitializationVector AESInitializationVector::createFromData(const std::vector<unsigned char>& data)
        {
            return AESInitializationVector(data);
        }

        AESInitializationVector::AESInitializationVector(bool random) :
            InitializationVector(DEFAULT_SIZE, random)
        {
        }

        AESInitializationVector::AESInitializationVector(const std::vector<unsigned char>& _data) :
            InitializationVector(_data)
        {
            assert(_data.size() == DEFAULT_SIZE);
        }
    }
}