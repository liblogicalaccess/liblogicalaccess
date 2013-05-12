/**
 * \file des_cipher.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief DES cipher class.
 */

#ifndef DES_CIPHER_HPP
#define DES_CIPHER_HPP

#include "logicalaccess/crypto/openssl_symmetric_cipher.hpp"

namespace logicalaccess
{
	namespace openssl
	{
		/**
		 * \brief A DES cipher.
		 *
		 * DESCipher is a symmetric cipher that relies on OpenSSL's implementation of the DES algorithm. Take a look at SymmetricCipher for usage.
		 */
		class DESCipher : public OpenSSLSymmetricCipher
		{
			public:

				/**
				 * \brief Constructor.
				 * \param _mode The encryption mode. Default is CBC.
				 */
				DESCipher(EncMode _mode = OpenSSLSymmetricCipher::ENC_MODE_CBC) : OpenSSLSymmetricCipher(_mode) {}

			protected:

				/**
				 * \brief Get the openssl EVP cipher.
				 * \param key The key.
				 * \return The EVP cipher.
				 */
				virtual const EVP_CIPHER* getEVPCipher(const SymmetricKey& key) const;
		};
	}
}

#endif /* AES_CIPHER_HPP */

