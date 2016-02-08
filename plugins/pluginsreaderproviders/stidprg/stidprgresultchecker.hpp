#pragma once

#include "logicalaccess/resultchecker.hpp"

namespace logicalaccess
{
class STidPRGResultChecker : public ResultChecker
{
  public:
    STidPRGResultChecker();

    virtual bool AllowEmptyResult() const override;
};
}