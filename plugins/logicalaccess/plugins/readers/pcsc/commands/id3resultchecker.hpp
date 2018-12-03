#pragma once

#include <logicalaccess/plugins/readers/iso7816/iso7816resultchecker.hpp>
#include <logicalaccess/plugins/readers/pcsc/lla_readers_pcsc_api.hpp>

namespace logicalaccess
{
class LLA_READERS_PCSC_API ID3ResultChecker : public ISO7816ResultChecker
{

  public:
    ID3ResultChecker();
};
}
