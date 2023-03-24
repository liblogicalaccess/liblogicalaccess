/**
 * \file samav3chip.hpp
 * \author Maxime C. <maxime@leosac.com>
 * \brief SAMAV3Chip source.
 */

#include <logicalaccess/plugins/cards/samav/samav3chip.hpp>

#include <cstring>

namespace logicalaccess
{
SAMAV3Chip::SAMAV3Chip(std::string ct)
    : SAMChip(ct)
{
}

SAMAV3Chip::SAMAV3Chip()
    : SAMChip(CHIP_SAMAV3)
{
}

SAMAV3Chip::~SAMAV3Chip()
{
}
}