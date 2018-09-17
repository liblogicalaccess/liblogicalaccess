/**
 * \file openssl.hpp
 * \author Julien K. <julien-dev@islog.com>
 * \brief OpenSSL global functions.
 */

#ifndef OPENSSL_HPP
#define OPENSSL_HPP

#include "logicalaccess/liblogicalaccess_export.hpp"

namespace logicalaccess
{
namespace openssl
{
class LIBLOGICALACCESS_API OpenSSLInitializer
{
  public:
    static OpenSSLInitializer &GetInstance()
    {
        static OpenSSLInitializer instance;
        return instance;
    }

  private:
    /**
     * \brief Initialize OpenSSL library.
     * \warning MUST be called in the main thread, at the very beginning of the
     * application.
     */
    OpenSSLInitializer();

    /**
    * \brief Remove copy.
    */
    OpenSSLInitializer(const OpenSSLInitializer &other) =
        delete; // non construction-copyable
    OpenSSLInitializer &operator=(const OpenSSLInitializer &) = delete; // non copyable

    /**
     * \brief Clean up OpenSSL library.
     * \warning MUST be called in the main thread, at the very end of the application.
     */
    ~OpenSSLInitializer();
};
}
}

#endif /* OPENSSL_HPP */