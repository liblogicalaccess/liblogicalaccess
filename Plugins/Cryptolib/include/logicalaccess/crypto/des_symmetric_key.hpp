/**
 * \file des_symmetric_key.hpp
 * \author Julien K. <julien-dev@islog.com>
 * \brief DES symmetric key class.
 */

#ifndef DES_SYMETRICKEY_HPP
#define DES_SYMETRICKEY_HPP

#include "logicalaccess/crypto/symmetric_key.hpp"

namespace logicalaccess
{
	namespace openssl
	{
		/**
		 * \brief An DES symmetric key.
		 *
		 * DESSymmetricKey represents a 8, 16 or 24 bytes long DES key to use with an instance DESCipher.
		 *
		 * You can create an DESSymmetricKey from random values, using createRandom() or from specific data, using createFromData().
		 */
		class DESSymmetricKey : public SymmetricKey
		{
			public:

				/**
				 * \brief Create a random symmetric key of a given length.
				 * \param size The size of the key. Must be 8, 16, or 24.
				 * \return The symmetric key.
				 */
				static DESSymmetricKey createRandom(size_t size);

				/**
				 * \brief Create a symmetric key from the specified data.
				 * \param data The data.
				 * \return The symmetric key.
				 * \warning If data is not 8, 16 or 24 bytes long, the behavior is undefined.
				 */
				static DESSymmetricKey createFromData(const std::vector<unsigned char>& data);

			protected:

				/**
				 * \brief Create a new random DESSymmetricKey.
				 * \param size The size of the key. Must be 8, 16, or 24.
				 */
				DESSymmetricKey(size_t size);

				/**
				 * \brief Create an DESInitializationVector from existing data.
				 * \param data The data.
				 * \warning If data is not 8, 16 or 24 bytes long, the behavior is undefined.
				 */
				DESSymmetricKey(const std::vector<unsigned char>& data);
		};
	}
}

#endif /* DES_SYMETRICKEY_HPP */

