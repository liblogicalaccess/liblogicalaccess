/**
 * \file openssl.cpp
 * \author Julien KAUFFMANN <julien.kauffmann@islog.eu>
 * \brief OpenSSL global functions.
 */

#include "logicalaccess/crypto/openssl.h"

#include <openssl/ssl.h>
#include <openssl/err.h>

namespace LOGICALACCESS
{
	namespace openssl
	{
		void initialize()
		{
			/* libssl initialization */
			SSL_library_init();

			/*  OpenSSL_add_all_algorithms(); */
			OpenSSL_add_all_digests();
			SSL_load_error_strings();
			ERR_load_crypto_strings();
			CRYPTO_malloc_init();
		}

		void cleanup()
		{
			/* cleanup SSL lib */

			EVP_cleanup();
			ERR_remove_state(0);
			ERR_free_strings();
			CRYPTO_cleanup_all_ex_data();
		}
	}
}

