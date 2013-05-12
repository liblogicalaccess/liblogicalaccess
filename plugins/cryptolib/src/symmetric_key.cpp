/**
 * \file symmetric_key.cpp
 * \author Julien K. <julien-dev@islog.com>
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Symmetric key class.
 */

#include "logicalaccess/crypto/symmetric_key.hpp"

#include <cstring>
#include <stdlib.h>

namespace logicalaccess
{
	namespace openssl
	{
		SymmetricKey::SymmetricKey(size_t size) :
			d_data(size)
		{
			randomize();
		}

		SymmetricKey::SymmetricKey(const std::vector<unsigned char>& _data) :
			d_data(_data)
		{
		}

		void SymmetricKey::randomize()
		{
			for (size_t i = 0; i < d_data.size(); ++i)
			{
				d_data[i] = static_cast<char>(rand() & 0xFF);
			}
		}
	}
}
