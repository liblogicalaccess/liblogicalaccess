/**
 * \file openssl_symmetric_cipher_context.cpp
 * \author Julien K. <julien-dev@islog.com>
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Open SSL symmetric cipher base class.
 */

#include <logicalaccess/plugins/crypto/openssl.hpp>
#include <logicalaccess/plugins/crypto/openssl_symmetric_cipher_context.hpp>
#include <logicalaccess/plugins/crypto/symmetric_key.hpp>
#include <assert.h>
#include <cstring>

namespace logicalaccess
{
namespace openssl
{
OpenSSLSymmetricCipherContext::Information::Information(
    OpenSSLSymmetricCipher::Method _method)
    : method(_method)
{
    ctx = EVP_CIPHER_CTX_new();
    assert(ctx && "Cannot allocate EVP cipher context.");
}

OpenSSLSymmetricCipherContext::Information::~Information()
{
    EVP_CIPHER_CTX_free(ctx);
}

OpenSSLSymmetricCipherContext::OpenSSLSymmetricCipherContext(
    OpenSSLSymmetricCipher::Method _method)
    : d_information(new Information(_method))
{
    OpenSSLInitializer::GetInstance();
}

void OpenSSLSymmetricCipherContext::setPadding(bool padding) const
{
    EVP_CIPHER_CTX_set_padding(d_information->ctx, padding ? 1 : 0);
}

size_t OpenSSLSymmetricCipherContext::blockSize() const
{
    assert(d_information);

    return EVP_CIPHER_CTX_block_size(d_information->ctx);
}

void OpenSSLSymmetricCipherContext::reset()
{
    d_information.reset();
}
}
}