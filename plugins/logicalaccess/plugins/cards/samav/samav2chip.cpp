/**
 * \file samav2chip.hpp
 * \author Adrien J. <adrien.jund@islog.com>
 * \brief SAMAV2Chip source.
 */

#include <logicalaccess/plugins/cards/samav/samav2chip.hpp>

#include <cstring>

namespace logicalaccess
{
SAMAV2Chip::SAMAV2Chip(std::string ct)
    : SAMChip(ct)
{
}

SAMAV2Chip::SAMAV2Chip()
    : SAMChip(CHIP_SAMAV2)
{
}

SAMAV2Chip::~SAMAV2Chip()
{
}
}