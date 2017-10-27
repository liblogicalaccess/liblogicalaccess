#pragma once

#include "logicalaccess/resultchecker.hpp"

namespace logicalaccess
{
class LIBLOGICALACCESS_API STidPRGResultChecker : public ResultChecker
{
  public:
    STidPRGResultChecker();

    bool AllowEmptyResult() const override;
};
}