/**
 * \file des_initialization_vector.cpp
 * \author Julien K. <julien-dev@islog.com>
 * \brief DES initialization vector class.
 */

#include "logicalaccess/crypto/des_initialization_vector.hpp"
#include <cassert>

namespace logicalaccess
{
    namespace openssl
    {
        DESInitializationVector DESInitializationVector::createNull()
        {
            return DESInitializationVector(false);
        }

        DESInitializationVector DESInitializationVector::createRandom()
        {
            return DESInitializationVector(true);
        }

        DESInitializationVector DESInitializationVector::createFromData(const ByteVector& data)
        {
            return DESInitializationVector(data);
        }

        DESInitializationVector::DESInitializationVector(bool random) :
            InitializationVector(DEFAULT_SIZE, random)
        {
        }

        DESInitializationVector::DESInitializationVector(const ByteVector& _data) :
            InitializationVector(_data)
        {
            assert(_data.size() == DEFAULT_SIZE);
        }
    }
}