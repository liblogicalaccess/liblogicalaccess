//
// Created by xaqq on 3/21/19.
//

#ifndef LOGICALACCESS_SEOSFORMAT_HPP
#define LOGICALACCESS_SEOSFORMAT_HPP

#include "logicalaccess/lla_core_api.hpp"

namespace logicalaccess
{

class SeosFormat
{
  public:
    SeosFormat() : dont_wrap_with_so(false) {}
    bool dont_wrap_with_so;
};

template <typename T>
class LLA_CORE_API SeosFormatT : public T, public SeosFormat
{
};

}

#endif // LOGICALACCESS_SEOSFORMAT_HPP
