#pragma once

#include <logicalaccess/myexception.hpp>

namespace logicalaccess
{
namespace iks
{
class RPCException : public LibLogicalAccessException
{
  public:
    explicit RPCException(const std::string &message)
        : LibLogicalAccessException(message)
    {
    }
};
};
}
