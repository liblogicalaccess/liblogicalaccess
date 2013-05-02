/**
 * \file symmetric_key.cpp
 * \author Julien KAUFFMANN <julien.kauffmann@islog.eu>
 * \author Maxime CHAMLEY <maxime.chamley@islog.eu>
 * \brief Symmetric key class.
 */

#include "logicalaccess/crypto/symmetric_key.h"

#include <cstring>
#include <stdlib.h>

namespace LOGICALACCESS
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
