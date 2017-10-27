/**
 * \file ssl_object.cpp
 * \author Julien K. <julien-dev@islog.com>
 * \brief A SSL object class.
 */

#include "logicalaccess/crypto/ssl_object.hpp"
#include "logicalaccess/crypto/ssl_context.hpp"
#include "logicalaccess/crypto/null_deleter.hpp"
#include <assert.h>

namespace logicalaccess
{
namespace openssl
{
void *SSLObject::getAppData() const
{
    return SSL_get_app_data(d_ssl.get());
}

void SSLObject::setAppData(void *data) const
{
    SSL_set_app_data(d_ssl.get(), data);
}

SSLContext SSLObject::sslContext() const
{
    SSL_CTX *ctx = SSL_get_SSL_CTX(d_ssl.get());

    return SSLContext(std::shared_ptr<SSL_CTX>(ctx, null_deleter()));
}

SSLObject::SSLObject(std::shared_ptr<SSL> ssl)
    : d_ssl(ssl)
{
    OpenSSLInitializer::GetInstance();
    assert(ssl);
}
}
}