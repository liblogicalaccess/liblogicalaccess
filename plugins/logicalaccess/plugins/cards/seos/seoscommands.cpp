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
SEOSCommands::~SEOSCommands() {}

std::shared_ptr<SEOSChip> SEOSCommands::getSEOSChip() const
{
    return std::dynamic_pointer_cast<SEOSChip>(getChip());
}
}