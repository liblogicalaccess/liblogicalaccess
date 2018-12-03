#pragma once

#include <logicalaccess/resultchecker.hpp>
#include <logicalaccess/plugins/readers/stidprg/lla_readers_stidprg_api.hpp>

namespace logicalaccess
{
class LLA_READERS_STIDPRG_API STidPRGResultChecker : public ResultChecker
{
  public:
    STidPRGResultChecker();

    bool AllowEmptyResult() const override;
};
}