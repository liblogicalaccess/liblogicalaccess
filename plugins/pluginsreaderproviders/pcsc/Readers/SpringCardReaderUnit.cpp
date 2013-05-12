/**
 * \file SpringCardReaderUnit.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief SpringCard reader unit.
 */

#include "../Readers/SpringCardReaderUnit.h"

#include <iostream>
#include <iomanip>
#include <sstream>


namespace logicalaccess
{
	SpringCardReaderUnit::SpringCardReaderUnit(const std::string& name)
		: PCSCReaderUnit(name)
	{
		
	}

	SpringCardReaderUnit::~SpringCardReaderUnit()
	{
		
	}

	PCSCReaderUnitType SpringCardReaderUnit::getPCSCType() const
	{
		return PCSC_RUT_SPRINGCARD;
	}
}
