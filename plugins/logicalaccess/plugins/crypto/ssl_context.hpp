/**
 * \file ssl_context.hpp
 * \author Julien K. <julien-dev@islog.com>
 * \brief A ssl context class.
 */

#ifndef SSL_CONTEXT_HPP
#define SSL_CONTEXT_HPP

#include <string>

#include <memory>

#ifdef _WINDOWS
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#undef X509_NAME
#undef X509_CERT_PAIR
#undef X509_EXTENSIONS
#endif
#include <openssl/ssl.h>

#include <logicalaccess/plugins/crypto/openssl.hpp>

namespace logicalaccess
{
namespace openssl
{
class X509Certificate;
class RSAKey;
class SecureSocket;

/**
 * \brief A SSL context.
 *
 * This class is used internally to represent a SSL context.
 */
class SSLContext
{
  public:
    /**
     * \brief Get the associated data.
     * \return The associated data.
     */
    void *getAppData() const;

    /**
     * \brief Set the associated data.
     * \param data The data.
     */
    void setAppData(void *data) const;

    /**
     * \brief Get the associated data.
     * \return The associated data.
     */
    template <typename T>
    T *getAppData()
    {
        return reinterpret_cast<T *>(getAppData());
    }

    /**
     * \brief Set the cipher list.
     * \param list The cipher list.
     */
    void setCipherList(const char *list) const;

    /**
     * \brief Set the "read-ahead" flag.
     * \param value true to set, false to unset.
     */
    void setReadAhead(bool value) const;

    /**
     * \brief Set the verify details.
     * \param mode The verify mode.
     * \param callback The verify callback.
     */
    void setVerify(int mode, int (*callback)(int, X509_STORE_CTX *)) const;

    /**
     * \brief Set the verify depth.
     * \param depth The depth.
     */
    void setVerifyDepth(int depth) const;

    /**
     * \brief Use a certificate.
     * \param cert The certificate to use. If cert is NULL, the behavior is undefined.
     */
    void useX509Certificate(const X509Certificate &cert) const;

    /**
     * \brief Use a private key (RSA).
     * \param key The RSA key to use.
     */
    void useRSAPrivateKey(const RSAKey &key) const;

    /**
     * \brief Load a verification certificate file.
     * \param verification_certificate The verification certificate.
     */
    void
    loadVerificationCertificate(const X509Certificate &verification_certificate) const;

    /**
     * \brief Load a verification certificate file.
     * \param verification_certificate_file The verification certificate file.
     */
    void loadVerificationCertificateFile(
        const std::string &verification_certificate_file) const;

    /**
     * \brief Load a verification certificate path.
     * \param verification_certificate_path The verification certificate path.
     */
    void loadVerificationCertificatePath(
        const std::string &verification_certificate_path) const;

    /**
     * \brief Get the raw pointer.
     * \return The raw pointer.
     */
    std::shared_ptr<SSL_CTX> getRaw() const
    {
        return d_ctx;
    }

  private:
    /**
     * \brief Create a null context.
     */
    SSLContext()
    {
        OpenSSLInitializer::GetInstance();
    }

    /**
     * \brief Constructor.
     * \param ctx The SSL context to use. If ctx is NULL, the behavior is undefined.
     */
    explicit SSLContext(std::shared_ptr<SSL_CTX> ctx);

    /**
     * \brief The boolean test.
     * \return true if d_ctx is not-null.
     */
    bool boolean_test() const
    {
        return bool(d_ctx);
    }

    /**
     * \brief Clear the context.
     */
    void clear()
    {
        d_ctx.reset();
    }

    /**
     * \brief The internal pointer.
     */
    std::shared_ptr<SSL_CTX> d_ctx;

    friend class SecureSocket;
    friend class SSLObject;
};
}
}

#endif /* SSL_CONTEXT_HPP */