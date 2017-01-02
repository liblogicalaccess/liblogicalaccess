/**
 * \file openssl.cpp
 * \author Julien K. <julien-dev@islog.com>
 * \brief OpenSSL global functions.
 */

#include "logicalaccess/crypto/openssl.hpp"

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include "logicalaccess/logs.hpp"
#include "logicalaccess/myexception.hpp"

namespace logicalaccess
{
    namespace openssl
    {
        OpenSSLInitializer::OpenSSLInitializer()
        {
            ERR_load_crypto_strings();
            OpenSSL_add_all_algorithms();
            CRYPTO_malloc_init();
        }

        OpenSSLInitializer::~OpenSSLInitializer()
        {
            CRYPTO_cleanup_all_ex_data();
            RAND_cleanup();
            EVP_cleanup();
            ERR_free_strings();
            ERR_remove_thread_state(0);
        }
    }
}
