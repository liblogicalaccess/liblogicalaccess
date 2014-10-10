/**
 * \file openssl_exception.hpp
 * \author Julien K. <julien-dev@islog.com>
 * \brief OpenSSL exception class.
 */

#ifndef OPENSSL_EXCEPTION_HPP
#define OPENSSL_EXCEPTION_HPP

#include <exception>
#include <string>
#include "logicalaccess/myexception.hpp"

namespace logicalaccess
{
    namespace openssl
    {
        /**
         * \brief A OpenSSL exception class.
         */
        class OpenSSLException : public Exception::exception
        {
        public:
            OpenSSLException(const std::string& message)
                : Exception::exception(message)
            {};
        };
    }
}

#endif /* OPENSSL_EXCEPTION_HPP */