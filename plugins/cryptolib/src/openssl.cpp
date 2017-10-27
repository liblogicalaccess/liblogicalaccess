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
#if OPENSSL_VERSION_NUMBER < 0x10100000L
    ERR_load_crypto_strings();
    OpenSSL_add_all_algorithms();
    CRYPTO_malloc_init();
#else
    OPENSSL_init_ssl(0, NULL);
    OpenSSL_add_all_algorithms();
#endif
}

OpenSSLInitializer::~OpenSSLInitializer()
{
    CRYPTO_cleanup_all_ex_data();
    RAND_cleanup();
    EVP_cleanup();
    ERR_free_strings();
#if OPENSSL_VERSION_NUMBER < 0x10100000L
    ERR_remove_thread_state(nullptr);
#endif
}
}
}
