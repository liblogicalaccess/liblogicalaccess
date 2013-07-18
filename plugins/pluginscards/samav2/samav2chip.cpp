/**
 * \file samav2chip.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief SAM AV2 chip.
 */

#include "samav2chip.hpp"

#include <cstring>

namespace logicalaccess
{
	SAMAV2Chip::SAMAV2Chip(std::string ct) : 
		Chip(ct)
	{
		
	}

	SAMAV2Chip::SAMAV2Chip() : 
		Chip(CHIP_SAMAV2)
	{
	}

	SAMAV2Chip::~SAMAV2Chip()
	{

	}
}
