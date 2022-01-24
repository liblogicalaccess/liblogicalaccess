//
// Created by xaqq on 13/01/2022.
//

#include "samav3iso7816commands.hpp"

namespace logicalaccess
{
void SAMAV3ISO7816Commands::yes_swig_we_are_abstract() {}

SAMAV3ISO7816Commands::SAMAV3ISO7816Commands()
    :
    SAMAV2ISO7816Commands(CMD_SAMAV3ISO7816)
{
}
}