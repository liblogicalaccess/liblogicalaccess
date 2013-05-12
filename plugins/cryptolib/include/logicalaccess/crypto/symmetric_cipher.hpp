/**
 * \file symmetric_cipher.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Symmetric cipher base class.
 */

#ifndef SYMETRICCIPHER_HPP
#define SYMETRICCIPHER_HPP

#include "logicalaccess/crypto/initialization_vector.hpp"

namespace logicalaccess
{
	class InitializationVector;

	namespace openssl
	{
		class SymmetricKey;

		/**
		 * \brief A symmetric cipher.
		 */
		class SymmetricCipher
		{
			public:

				/**
				 * \brief Destructor.
				 */
				virtual ~SymmetricCipher() {};

				/**
				 * \brief Cipher a buffer.
				 * \param src The buffer to cipher.
				 * \param dest The ciphered buffer.
				 * \param key The key to use.
				 * \param iv The initialisation vector.
				 * \param padding Whether to use padding.
				 * In case of a failure, the call throw an InvalidCallException.
				 */
				virtual void cipher(const std::vector<unsigned char>& src, std::vector<unsigned char>& dest, const SymmetricKey& key, const InitializationVector& iv, bool padding) = 0;

				/**
				 * \brief Decipher a buffer.
				 * \param src The buffer to decipher.
				 * \param dest The deciphered buffer.
				 * \param key The key to use.
				 * \param iv The initialisation vector.
				 * \param padding Whether to use padding.
				 * In case of a failure, the call throw a InvalidCallException.
				 */
				virtual void decipher(const std::vector<unsigned char>& src, std::vector<unsigned char>& dest, const SymmetricKey& key, const InitializationVector& iv, bool padding) = 0;
		};
	}
}

#endif /* SYMETRICCIPHER_HPP */

