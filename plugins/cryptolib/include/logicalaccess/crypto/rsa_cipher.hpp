/**
 * \file rsa_cipher.hpp
 * \author Julien K. <julien-dev@islog.com>
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief RSA cipher class.
 */

#ifndef RSA_CIPHER_HPP
#define RSA_CIPHER_HPP

#include "logicalaccess/crypto/openssl_asymmetric_cipher.hpp"

namespace logicalaccess
{
    namespace openssl
    {
        /**
         * \brief A RSA cipher.
         * \see AsymmetricCipher
         * \see OpenSSLAsymmetricCipher
         *
         * See AsymmetricCipher for usage.
         */
        class RSACipher : public OpenSSLAsymmetricCipher
        {
        public:

            /**
             * \brief Cipher a buffer.
             * \param src The buffer to cipher.
             * \param dest The ciphered buffer.
             * \param key The key to use. If key is not an instance of RSAKey, the behavior is undefined.
             * \param key_compound The key compound to use.
             * In case of a failure, the call throw an InvalidCallException.
             */
	        void cipher(const ByteVector& src, ByteVector& dest, const AsymmetricKey& key, KeyCompound key_compound) override;

            /**
             * \brief Decipher a buffer.
             * \param src The buffer to decipher.
             * \param dest The deciphered buffer.
             * \param key The key to use. If key is not an instance of RSAKey, the behavior is undefined.
             * \param key_compound The key compound to use.
             * In case of a failure, the call throw a InvalidCallException.
             */
	        void decipher(const ByteVector& src, ByteVector& dest, const AsymmetricKey& key, KeyCompound key_compound) override;
        };
    }
}

#endif /* RSA_CIPHER_HPP */