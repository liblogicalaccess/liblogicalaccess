/**
 * \file openssl_asymmetric_cipher.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@islog.eu>
 * \author Maxime CHAMLEY <maxime.chamley@islog.eu>
 * \brief OpenSSL asymmetric cipher base class.
 */

#ifndef OPENSSLASYMETRICCIPHER_HPP
#define OPENSSLASYMETRICCIPHER_HPP

#include "logicalaccess/crypto/asymmetric_cipher.h"

namespace LOGICALACCESS
{
	namespace openssl
	{
		/**
		 * \brief The base class for all OpenSSL based asymmetric ciphers.
		 * \see RSACipher
		 * \see AsymmetricCipher
		 */
		class OpenSSLAsymmetricCipher : public AsymmetricCipher
		{
		};
	}
}

#endif /* OPENSSLASYMETRICCIPHER_HPP */

