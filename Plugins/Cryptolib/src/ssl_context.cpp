/**
 * \file ssl_context.cpp
 * \author Julien K. <julien-dev@islog.com>
 * \brief A SSL context class.
 */

#include "logicalaccess/crypto/ssl_context.h"
#include "logicalaccess/x509_certificate.h"
#include "logicalaccess/crypto/rsa_key.h"
#include "logicalaccess/logs.h"
#include "logicalaccess/crypto/openssl_exception.h"

namespace logicalaccess
{
	namespace openssl
	{
		void* SSLContext::getAppData()
		{
			return SSL_CTX_get_app_data(d_ctx.get());
		}

		void SSLContext::setAppData(void* data)
		{
			SSL_CTX_set_app_data(d_ctx.get(), data);
		}

		void SSLContext::setCipherList(const char* list)
		{
			EXCEPTION_ASSERT_WITH_LOG(SSL_CTX_set_cipher_list(d_ctx.get(), list) == 1, OpenSSLException, "Cannot set the SSL context cipher list");
		}

		void SSLContext::setReadAhead(bool value)
		{
			SSL_CTX_set_read_ahead(d_ctx.get(), (value ? 1 : 0));
		}

		void SSLContext::setVerify(int mode, int (*callback)(int, X509_STORE_CTX*))
		{
			SSL_CTX_set_verify(d_ctx.get(), mode, callback);
		}

		void SSLContext::setVerifyDepth(int depth)
		{
			SSL_CTX_set_verify_depth(d_ctx.get(), depth);
		}

		void SSLContext::useX509Certificate(const X509Certificate& cert)
		{
			EXCEPTION_ASSERT_WITH_LOG(SSL_CTX_use_certificate(d_ctx.get(), cert.getRaw().get()) == 1, OpenSSLException, "Cannot load the certificate");
		}

		void SSLContext::useRSAPrivateKey(const RSAKey& key)
		{
			EXCEPTION_ASSERT_WITH_LOG(SSL_CTX_use_RSAPrivateKey(d_ctx.get(), key.getRaw().get()) == 1, OpenSSLException, "Cannot load the private key (does the certificate match ?)");
		}

		void SSLContext::loadVerificationCertificate(const X509Certificate& verification_certificate)
		{
			X509* pcert = X509_dup(verification_certificate.getRaw().get());

			EXCEPTION_ASSERT_WITH_LOG(SSL_CTX_add_extra_chain_cert(d_ctx.get(), pcert) == 1, OpenSSLException, "Unable to load verification certificate");
		}

		void SSLContext::loadVerificationCertificateFile(const std::string& verification_certificate_file)
		{
			EXCEPTION_ASSERT_WITH_LOG(SSL_CTX_load_verify_locations(d_ctx.get(), verification_certificate_file.c_str(), NULL) == 1, OpenSSLException, "Unable to load the verification certificate file");
		}

		void SSLContext::loadVerificationCertificatePath(const std::string& verification_certificate_path)
		{
			EXCEPTION_ASSERT_WITH_LOG(SSL_CTX_load_verify_locations(d_ctx.get(), NULL, verification_certificate_path.c_str()) == 1, OpenSSLException, "Unable to load the verification certificate path");
		}

		SSLContext::SSLContext(boost::shared_ptr<SSL_CTX> ctx) :
			d_ctx(ctx)
		{
			assert(ctx);
		}
	}
}

