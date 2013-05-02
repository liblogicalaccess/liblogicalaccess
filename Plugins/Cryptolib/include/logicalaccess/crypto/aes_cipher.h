/**
 * \file aes_cipher.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@islog.eu>
 * \author Maxime CHAMLEY <maxime.chamley@islog.eu>
 * \brief AES cipher class.
 */

#ifndef AES_CIPHER_HPP
#define AES_CIPHER_HPP

#include "logicalaccess/crypto/openssl_symmetric_cipher.h"

namespace LOGICALACCESS
{
	namespace openssl
	{
		/**
		 * \brief AES cipher class.
		 *
		 * AESCipher is a symmetric cipher class that relies on OpenSSL's implementation of the AES algorithm. Take a look at SymmetricCipher for usage.
		 */
		class AESCipher : public OpenSSLSymmetricCipher
		{
			public:

				/**
				 * \brief Constructor.
				 * \param _mode The encryption mode. Default is CBC.
				 */
				AESCipher(EncMode _mode = OpenSSLSymmetricCipher::ENC_MODE_CBC) : OpenSSLSymmetricCipher(_mode) {}

			protected:

				/**
				 * \brief Get the OpenSSL EVP cipher.
				 * \param key The key.
				 * \return The EVP cipher.
				 *
				 * getEVPCipher() returns the OpenSSL EVP cipher that should be used with key. The resulting EVP cipher mainly depends on the size of key.
				 *
				 * If no suitable cipher is found, getEVPCipher() returns NULL.
				 */
				virtual const EVP_CIPHER* getEVPCipher(const SymmetricKey& key) const;
		};
	}
}

#endif /* AES_CIPHER_HPP */

