/**
 * \file des_initialization_vector.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@islog.eu>
 * \brief DES initialization vector class.
 */

#ifndef DES_INITIALIZATION_VECTOR_HPP
#define DES_INITIALIZATION_VECTOR_HPP

#include "logicalaccess/crypto/initialization_vector.h"

namespace LOGICALACCESS
{
	namespace openssl
	{
		/**
		 * \brief An DES initialization vector.
		 *
		 * DESInitializationVector represents an DES initialization vector (IV). Every DESInitializationVector is 8 bytes long.
		 *
		 * It is meant to be used with an instance of DESCipher.
		 */
		class DESInitializationVector : public InitializationVector
		{
			public:

				/**
				 * \brief Create a null IV.
				 */
				static DESInitializationVector createNull();

				/**
				 * \brief Create a random IV.
				 */
				static DESInitializationVector createRandom();

				/**
				 * \brief Create an IV from data.
				 * \param data The data. Must be 8 exactly bytes long.
				 * \return IV from the data.
				 * \warning If data is not exactly 8 bytes long, the behavior is undefined.
				 */
				static DESInitializationVector createFromData(const std::vector<unsigned char>& data);

			protected:

				/**
				 * \brief Create a new DESInitializationVector.
				 * \param random true if the IV must be randomized, false otherwise.
				 */
				DESInitializationVector(bool random);

				/**
				 * \brief Create an DESInitializationVector from existing data.
				 * \param data The data.
				 * \warning If data is not 8 bytes long, the behavior is undefined.
				 */
				DESInitializationVector(const std::vector<unsigned char>& data);

			private:

				/**
				 * \brief The DES initialization vector size.
				 */
				static const size_t DEFAULT_SIZE = 8;
		};
	}
}

#endif /* DES_INITIALIZATION_VECTOR_HPP */

