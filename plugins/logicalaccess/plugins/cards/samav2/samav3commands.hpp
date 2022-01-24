#pragma once

#include <logicalaccess/plugins/cards/samav2/samav2commands.hpp>

namespace logicalaccess
{

class LLA_CARDS_SAMAV2_API SAMAV3Commands :
    public virtual SAMAV2Commands<KeyEntryAV2Information, SETAV2>
{
  public:

    /**
     * A hack because i think swig was misdetecting the class as non-abstract.
     */
    virtual void yes_swig_we_are_abstract() = 0;
};
}
