/**
 * \file SpringCardReaderUnit.cpp
 * \author Maxime CHAMLEY <maxime.chamley@islog.eu>
 * \brief SpringCard reader unit.
 */

#include "../Readers/SpringCardReaderUnit.h"

#include <iostream>
#include <iomanip>
#include <sstream>


namespace LOGICALACCESS
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
