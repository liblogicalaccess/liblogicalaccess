/**
 * \file openssl_symmetric_cipher.cpp
 * \author Julien K. <julien-dev@islog.com>
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Open SSL symmetric cipher base class.
 */

#include <stdexcept>
#include "logicalaccess/crypto/openssl_symmetric_cipher.hpp"
#include "logicalaccess/crypto/openssl_symmetric_cipher_context.hpp"
#include "logicalaccess/crypto/symmetric_key.hpp"
#include "logicalaccess/logs.hpp"
#include "logicalaccess/myexception.hpp"
#include "logicalaccess/crypto/openssl_exception.hpp"
#include "logicalaccess/crypto/sha.hpp"

#include <cstring>

namespace logicalaccess
{
	namespace openssl
	{
		OpenSSLSymmetricCipher::OpenSSLSymmetricCipher(OpenSSLSymmetricCipher::EncMode _mode) :
			d_mode(_mode)
		{
		}

		OpenSSLSymmetricCipherContext OpenSSLSymmetricCipher::start(OpenSSLSymmetricCipher::Method method, const SymmetricKey& key, const InitializationVector& iv, bool padding)
		{
			OpenSSLSymmetricCipherContext context(method);

			int r = 0;

			const EVP_CIPHER* evpCipher = getEVPCipher(key);

			EXCEPTION_ASSERT(evpCipher, std::invalid_argument, "No cipher found that can use the supplied key");

			switch (context.method())
			{
				case M_ENCRYPT:
					{
						r = EVP_EncryptInit_ex(context.ctx(), evpCipher, NULL, &key.data()[0], &iv.data()[0]);
						break;
					}
				case M_DECRYPT:
					{
						r = EVP_DecryptInit_ex(context.ctx(), evpCipher, NULL, &key.data()[0], &iv.data()[0]);
						break;
					}
				default:
					{
						throw Exception::exception("Unhandled method");
					}
			}

			EXCEPTION_ASSERT_WITH_LOG(r == 1, OpenSSLException, "");

			context.data().clear();

			context.setPadding(padding);

			return context;
		}

		void OpenSSLSymmetricCipher::update(OpenSSLSymmetricCipherContext& context, const std::vector<unsigned char>& src)
		{
			int r = 0;
			int outlen = 0;

			unsigned char* buf = new unsigned char[src.size() + context.blockSize()];

			switch (context.method())
			{
				case M_ENCRYPT:
					{
						r = EVP_EncryptUpdate(context.ctx(), buf, &outlen, &src[0], static_cast<int>(src.size()));
						break;
					}
				case M_DECRYPT:
					{
						r = EVP_DecryptUpdate(context.ctx(), buf, &outlen, &src[0], static_cast<int>(src.size()));
						break;
					}
				default:
					{
						throw Exception::exception("Unhandled method");
					}
			}

			if (r != 1)
			{
				delete[] buf;
				THROW_EXCEPTION_WITH_LOG(OpenSSLException, "");
			}

			context.data().insert(context.data().end(), buf, buf + outlen);
			delete[] buf;
		}

		std::vector<unsigned char> OpenSSLSymmetricCipher::stop(OpenSSLSymmetricCipherContext& context)
		{
			int r = 0;
			int outlen = 0;

			unsigned char* buf = new unsigned char[context.blockSize()];

			switch (context.method())
			{
				case M_ENCRYPT:
					{
						r = EVP_EncryptFinal_ex(context.ctx(), buf, &outlen);
						break;
					}
				case M_DECRYPT:
					{
						r = EVP_DecryptFinal_ex(context.ctx(), buf, &outlen);
						break;
					}
				default:
					{
						throw Exception::exception("Unhandled method");
					}
			}

			if (r != 1)
			{
				delete[] buf;
				THROW_EXCEPTION_WITH_LOG(OpenSSLException, "");
			}

			context.data().insert(context.data().end(), buf, buf + outlen);
			delete[] buf;

			std::vector<unsigned char> data = context.data();

			context.reset();

			return data;
		}

		void OpenSSLSymmetricCipher::cipher(const std::vector<unsigned char>& src, std::vector<unsigned char>& dest, const SymmetricKey& key, const InitializationVector& iv, bool padding)
		{
			OpenSSLSymmetricCipherContext context = start(M_ENCRYPT, key, iv, padding);

			update(context, src);

			dest = stop(context);
		}

		void OpenSSLSymmetricCipher::decipher(const std::vector<unsigned char>& src, std::vector<unsigned char>& dest, const SymmetricKey& key, const InitializationVector& iv, bool padding)
		{
			OpenSSLSymmetricCipherContext context = start(M_DECRYPT, key, iv, padding);

			update(context, src);

			dest = stop(context);
		}
	}
}