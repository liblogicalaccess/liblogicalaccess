/**
 * \file openssl_asymmetric_cipher.hpp
 * \author Julien K. <julien-dev@islog.com>
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief OpenSSL asymmetric cipher base class.
 */

#ifndef OPENSSLASYMETRICCIPHER_HPP
#define OPENSSLASYMETRICCIPHER_HPP

#include "logicalaccess/crypto/asymmetric_cipher.hpp"

namespace logicalaccess
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

