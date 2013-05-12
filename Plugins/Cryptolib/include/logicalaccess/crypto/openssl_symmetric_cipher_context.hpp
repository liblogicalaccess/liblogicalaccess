/**
 * \file openssl_symmetric_cipher_context.hpp
 * \author Julien K. <julien-dev@islog.com>
 * \brief OpenSSL symmetric cipher context base class.
 */

#ifndef OPENSSL_SYMETRIC_CIPHER_CONTEXT_HPP
#define OPENSSL_SYMETRIC_CIPHER_CONTEXT_HPP

#include "logicalaccess/crypto/openssl_symmetric_cipher.hpp"
#include <boost/shared_ptr.hpp>

#include <openssl/evp.h>

#include <boost/noncopyable.hpp>

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
			public:

			private:

				/**
				 * \brief The context information structure.
				 */
				struct Information : public boost::noncopyable
				{
					/**
					 * \brief Constructor.
					 * \param method The method.
					 */
					Information(OpenSSLSymmetricCipher::Method method);

					/**
					 * \brief Destructor.
					 */
					~Information();

					/**
					 * \brief The internal OpenSSL context.
					 */
					EVP_CIPHER_CTX ctx;

					/**
					 * \brief The internal method.
					 */
					OpenSSLSymmetricCipher::Method method;

					/**
					 * \brief An internal buffer.
					 */
					std::vector<unsigned char> data;
				};

				/**
				 * \brief Constructor.
				 * \param method The method.
				 */
				OpenSSLSymmetricCipherContext(OpenSSLSymmetricCipher::Method method);

				/**
				 * \brief Set the padding.
				 * \param padding true to enable padding, false to disable it.
				 *
				 * Padding is disabled by default.
				 */
				void setPadding(bool padding);

				/**
				 * \brief Get the data.
				 * \return The data.
				 */
				EVP_CIPHER_CTX* ctx();

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
				std::vector<unsigned char>& data();

				/**
				 * \brief Reset the context.
				 */
				void reset();

				/**
				 * \brief The information.
				 */
				boost::shared_ptr<Information> d_information;

				friend class OpenSSLSymmetricCipher;
		};
		
		inline EVP_CIPHER_CTX* OpenSSLSymmetricCipherContext::ctx() { return &d_information->ctx; }
		inline OpenSSLSymmetricCipher::Method OpenSSLSymmetricCipherContext::method() const { return d_information->method; }
		inline std::vector<unsigned char>& OpenSSLSymmetricCipherContext::data() { return d_information->data; }
	}
}

#endif /* OPENSSL_SYMETRIC_CIPHER_CONTEXT_HPP */

