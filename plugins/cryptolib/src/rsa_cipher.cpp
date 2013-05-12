/**
 * \file rsa_cipher.cpp
 * \author Julien K. <julien-dev@islog.com>
 * \brief RSA cipher class.
 */

#include "logicalaccess/crypto/rsa_cipher.hpp"
#include "logicalaccess/crypto/rsa_key.hpp"
#include "logicalaccess/logs.hpp"
#include "logicalaccess/crypto/openssl_exception.hpp"

#include <cstring>

#include <openssl/evp.h>
#include <openssl/buffer.h>
#include <openssl/err.h>
#include <openssl/bio.h>
#include <openssl/ssl.h>

namespace logicalaccess
{
	namespace openssl
	{
		void RSACipher::cipher(const std::vector<unsigned char>& src, std::vector<unsigned char>& dest, const AsymmetricKey& key, KeyCompound key_compound)
		{
			const RSAKey& rsakey = static_cast<const RSAKey&>(key);

			int len = 0;
			size_t sumlen = 0;
			size_t blen = RSA_size(rsakey.d_rsa.get()) - 11;

			size_t c = (src.size() / blen) + 1;

			dest.resize(RSA_size(rsakey.d_rsa.get()) * c);

			for (size_t offset = 0; offset < src.size(); offset += blen)
			{
				if (blen + offset > src.size())
				{
					blen = src.size() - offset;
				}

				if (key_compound == KC_PUBLIC)
				{
					len = RSA_public_encrypt(static_cast<int>(blen), &src[offset], &dest[sumlen], rsakey.d_rsa.get(), RSA_PKCS1_PADDING);
				}
				else
				{
					len = RSA_private_encrypt(static_cast<int>(blen), &src[offset], &dest[sumlen], rsakey.d_rsa.get(), RSA_PKCS1_PADDING);
				}

				EXCEPTION_ASSERT_WITH_LOG(len >= 0, OpenSSLException, "RSA public encrypt failed");

				sumlen += len;
			}

			dest.resize(sumlen);
		}

		void RSACipher::decipher(const std::vector<unsigned char>& src, std::vector<unsigned char>& dest, const AsymmetricKey& key, KeyCompound key_compound)
		{
			const RSAKey& rsakey = static_cast<const RSAKey&>(key);

			int len = 0;
			size_t sumlen = 0;
			size_t blen = RSA_size(rsakey.d_rsa.get());

			size_t c = (src.size() / blen) + 1;

			dest.resize(RSA_size(rsakey.d_rsa.get()) * c);

			for (size_t offset = 0; offset < src.size(); offset += blen)
			{
				if (blen + offset > src.size())
				{
					blen = src.size() - offset;
				}

				if (key_compound == KC_PUBLIC)
				{
					len = RSA_public_decrypt(static_cast<int>(blen), &src[offset], &dest[sumlen], rsakey.d_rsa.get(), RSA_PKCS1_PADDING);
				}
				else
				{
					len = RSA_private_decrypt(static_cast<int>(blen), &src[offset], &dest[sumlen], rsakey.d_rsa.get(), RSA_PKCS1_PADDING);
				}

				EXCEPTION_ASSERT_WITH_LOG(len >= 0, OpenSSLException, "RSA public decrypt failed");

				sumlen += len;
			}

			dest.resize(sumlen);
		}
	}
}

