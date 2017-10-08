/**
 * \file openssl_symmetric_cipher_context.hpp
 * \author Julien K. <julien-dev@islog.com>
 * \brief OpenSSL symmetric cipher context base class.
 */

#ifndef OPENSSL_SYMETRIC_CIPHER_CONTEXT_HPP
#define OPENSSL_SYMETRIC_CIPHER_CONTEXT_HPP

#include "logicalaccess/crypto/openssl_symmetric_cipher.hpp"
#include <memory>

#include <openssl/evp.h>

namespace logicalaccess
{
    namespace openssl
    {
        /**
         * \brief An OpenSSL context class.
         *
         * The Context class is used by OpenSSLSymmetricCipher to perform block cipherement. It cannot be publicly instanciated.
         */
        class OpenSSLSymmetricCipherContext
        {
	        /**
             * \brief The context information structure.
             */
            struct Information
            {
                /**
                 * \brief Constructor.
                 * \param method The method.
                 */
	            explicit Information(OpenSSLSymmetricCipher::Method method);

                /**
                 * \brief Destructor.
                 */
                ~Information();

				/**
				* \brief Remove copy.
				*/
				Information(const Information& other) = delete; // non construction-copyable
				Information& operator=(const Information&) = delete; // non copyable

                /**
                 * \brief The internal OpenSSL context.
                 */
                EVP_CIPHER_CTX *ctx;

                /**
                 * \brief The internal method.
                 */
                OpenSSLSymmetricCipher::Method method;

                /**
                 * \brief An internal buffer.
                 */
                ByteVector data;
            };

            /**
             * \brief Constructor.
             * \param method The method.
             */
	        explicit OpenSSLSymmetricCipherContext(OpenSSLSymmetricCipher::Method method);

            /**
             * \brief Set the padding.
             * \param padding true to enable padding, false to disable it.
             *
             * Padding is disabled by default.
             */
            void setPadding(bool padding) const;

            /**
             * \brief Get the data.
             * \return The data.
             */
            EVP_CIPHER_CTX* ctx() const;

            /**
             * \brief Get the block size.
             * \return The block size.
             */
            size_t blockSize() const;

            /**
             * \brief Get the method.
             * \return The method.
             */
            OpenSSLSymmetricCipher::Method method() const;

            /**
             * \brief Get the data.
             * \return The data.
             */
            ByteVector& data() const;

            /**
             * \brief Reset the context.
             */
            void reset();

            /**
             * \brief The information.
             */
            std::shared_ptr<Information> d_information;

            friend class OpenSSLSymmetricCipher;
        };

        inline EVP_CIPHER_CTX* OpenSSLSymmetricCipherContext::ctx() const { return d_information->ctx; }
        inline OpenSSLSymmetricCipher::Method OpenSSLSymmetricCipherContext::method() const { return d_information->method; }
        inline ByteVector& OpenSSLSymmetricCipherContext::data() const { return d_information->data; }
    }
}

#endif /* OPENSSL_SYMETRIC_CIPHER_CONTEXT_HPP */