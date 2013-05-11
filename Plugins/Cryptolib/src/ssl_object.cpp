/**
 * \file ssl_object.cpp
 * \author Julien K. <julien-dev@islog.com>
 * \brief A SSL object class.
 */

#include "logicalaccess/crypto/ssl_object.h"
#include "logicalaccess/crypto/ssl_context.h"
#include "logicalaccess/crypto/null_deleter.h"

namespace logicalaccess
{
	namespace openssl
	{
		void* SSLObject::getAppData()
		{
			return SSL_get_app_data(d_ssl.get());
		}

		void SSLObject::setAppData(void* data)
		{
			SSL_set_app_data(d_ssl.get(), data);
		}

		SSLContext SSLObject::sslContext()
		{
			SSL_CTX* ctx = SSL_get_SSL_CTX(d_ssl.get());

			return SSLContext(boost::shared_ptr<SSL_CTX>(ctx, null_deleter()));
		}

		SSLObject::SSLObject(boost::shared_ptr<SSL> ssl) :
			d_ssl(ssl)
		{
			assert(ssl);
		}
	}
}

