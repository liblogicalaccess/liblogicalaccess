/**
 * \file openssl_exception.hpp
 * \author Julien K. <julien-dev@islog.com>
 * \brief OpenSSL exception class.
 */

#ifndef OPENSSL_EXCEPTION_HPP
#define OPENSSL_EXCEPTION_HPP

#include <exception>
#include <string>
#include <logicalaccess/myexception.hpp>

namespace logicalaccess
{
namespace openssl
{
/**
 * \brief A OpenSSL exception class.
 */
class OpenSSLException : public LibLogicalAccessException
{
  public:
    explicit OpenSSLException(const std::string &message)
        : LibLogicalAccessException(message)
    {
    }
};
}
}

#endif /* OPENSSL_EXCEPTION_HPP */