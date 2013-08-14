/**
 * \file samav2chip.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief SAM AV2 chip.
 */

#include "samav1chip.hpp"

#include <cstring>

namespace logicalaccess
{
	SAMAV1Chip::SAMAV1Chip(std::string ct) : 
		SAMChip(ct)
	{
		
	}

	SAMAV1Chip::SAMAV1Chip() : 
		SAMChip(CHIP_SAMAV1)
	{
	}

	SAMAV1Chip::~SAMAV1Chip()
	{

	}
}
