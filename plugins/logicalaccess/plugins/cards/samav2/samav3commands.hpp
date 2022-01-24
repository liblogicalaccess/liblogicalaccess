#pragma once

#include <logicalaccess/plugins/cards/samav2/samav2commands.hpp>

namespace logicalaccess
{

class LLA_CARDS_SAMAV2_API SAMAV3Commands :
    public virtual SAMAV2Commands<KeyEntryAV2Information, SETAV2>
{
  public:

    /**
     * Check if the sam is activated or not.
     * An underlying getVersion command is used.
     */
    virtual bool ext_IsActivated() = 0;

    /**
     * Activate the SAM.
     * This is the initial SAM activation to make sure
     * the sam was not tempered with before.
     */
    virtual void ext_ActivateSAM() = 0 ;


    virtual std::vector<uint8_t> getRandom(uint8_t random_size) = 0;

    /**
     * A hack because i think swig was misdetecting the class as non-abstract.
     */
    virtual void yes_swig_we_are_abstract() = 0;
};
}
