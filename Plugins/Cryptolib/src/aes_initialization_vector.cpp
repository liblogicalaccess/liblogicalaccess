/**
 * \file aes_initialization_vector.cpp
 * \author Julien KAUFFMANN <julien.kauffmann@islog.eu>
 * \author Maxime CHAMLEY <maxime.chamley@islog.eu>
 * \brief AES initialization vector class.
 */

#include "logicalaccess/crypto/aes_initialization_vector.h"
#include <cassert>

namespace LOGICALACCESS
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
