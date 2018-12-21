/**
 * \file seoscommands.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief SEOS commands.
 */

#include <logicalaccess/plugins/cards/seos/seoscommands.hpp>
#include <logicalaccess/plugins/cards/seos/seoschip.hpp>
#include <cstring>

namespace logicalaccess
{
SeosCommands::~SeosCommands() {}

std::shared_ptr<SeosChip> SeosCommands::getSeosChip() const
{
    return std::dynamic_pointer_cast<SeosChip>(getChip());
}
}