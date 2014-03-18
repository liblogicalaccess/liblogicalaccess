/**
 * \file samav1chip.cpp
 * \author Adrien J. <adrien.jund@islog.com>
 * \brief SAMAV1Chip source.
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
