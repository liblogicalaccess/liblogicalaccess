/**
 * \file openssl_symmetric_cipher_context.cpp
 * \author Julien KAUFFMANN <julien.kauffmann@islog.eu>
 * \author Maxime CHAMLEY <maxime.chamley@islog.eu>
 * \brief Open SSL symmetric cipher base class.
 */

#include "logicalaccess/crypto/openssl_symmetric_cipher_context.h"
#include "logicalaccess/crypto/symmetric_key.h"
#include "logicalaccess/crypto/sha.h"

#include <cstring>

namespace LOGICALACCESS
{
	namespace openssl
	{
		OpenSSLSymmetricCipherContext::Information::Information(OpenSSLSymmetricCipher::Method _method) :
			method(_method)
		{
			EVP_CIPHER_CTX_init(&ctx);
		}

		OpenSSLSymmetricCipherContext::Information::~Information()
		{
			EVP_CIPHER_CTX_cleanup(&ctx);
		}

		OpenSSLSymmetricCipherContext::OpenSSLSymmetricCipherContext(OpenSSLSymmetricCipher::Method _method) :
			d_information(new Information(_method))
		{
		}

		void OpenSSLSymmetricCipherContext::setPadding(bool padding)
		{
			EVP_CIPHER_CTX_set_padding(&d_information->ctx, padding ? 1 : 0);
		}

		size_t OpenSSLSymmetricCipherContext::blockSize() const
		{
			assert(d_information);

			return EVP_CIPHER_CTX_block_size(&d_information->ctx);
		}

		void OpenSSLSymmetricCipherContext::reset()
		{
			d_information.reset();
		}
	}
}
