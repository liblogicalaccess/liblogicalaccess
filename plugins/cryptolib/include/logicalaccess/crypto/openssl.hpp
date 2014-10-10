/**
 * \file openssl.hpp
 * \author Julien K. <julien-dev@islog.com>
 * \brief OpenSSL global functions.
 */

#ifndef OPENSSL_HPP
#define OPENSSL_HPP

#include <boost/noncopyable.hpp>

namespace logicalaccess
{
    namespace openssl
    {
        class OpenSSLInitializer : public boost::noncopyable
        {
        public:
            static OpenSSLInitializer& GetInstance()
            {
                static OpenSSLInitializer instance;
                return instance;
            }

        private:

            /**
             * \brief Initialize OpenSSL library.
             * \warning MUST be called in the main thread, at the very beginning of the application.
             */
            OpenSSLInitializer();

            /**
             * \brief Clean up OpenSSL library.
             * \warning MUST be called in the main thread, at the very end of the application.
             */
            ~OpenSSLInitializer();
        };
    }
}

#endif /* OPENSSL_HPP */