#pragma once

#include "logicalaccess/plugins/readers/iso7816/lla_readers_iso7816_api.hpp"
#include "logicalaccess/plugins/readers/iso7816/commands/samav2iso7816commands.hpp"
#include <logicalaccess/plugins/cards/samav2/samav3commands.hpp>

namespace logicalaccess
{
#define CMD_SAMAV3ISO7816 "SAMAV3ISO7816"

class LLA_READERS_ISO7816_API SAMAV3ISO7816Commands
    :
      public SAMAV2ISO7816Commands,
      public virtual SAMAV3Commands
{
  public:
    SAMAV3ISO7816Commands();

    using KeyEntryT = SAMKeyEntry<KeyEntryAV2Information, SETAV2>;
    void yes_swig_we_are_abstract() override;
};
}
