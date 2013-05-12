/**
 * \file des_symmetric_key.cpp
 * \author Julien K. <julien-dev@islog.com>
 * \brief DES symmetric key class.
 */

#include "logicalaccess/crypto/des_symmetric_key.hpp"

namespace logicalaccess
{
	namespace openssl
	{
		DESSymmetricKey DESSymmetricKey::createRandom(size_t size)
		{
			return DESSymmetricKey(size);
		}

		DESSymmetricKey DESSymmetricKey::createFromData(const std::vector<unsigned char>& data)
		{
			return DESSymmetricKey(data);
		}

		DESSymmetricKey::DESSymmetricKey(size_t size) :
			SymmetricKey(size)
		{
		}

		DESSymmetricKey::DESSymmetricKey(const std::vector<unsigned char>& _data) :
			SymmetricKey(_data)
		{
		}
	}
}
