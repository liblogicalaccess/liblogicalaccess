/**
 * \file CherryReaderUnit.cpp
 * \author Maxime CHAMLEY <maxime.chamley@islog.eu>
 * \brief Cherry reader unit.
 */

#include "../Readers/CherryReaderUnit.h"

#include <iostream>
#include <iomanip>
#include <sstream>


namespace LOGICALACCESS
{
	CherryReaderUnit::CherryReaderUnit(const std::string& name)
		: PCSCReaderUnit(name)
	{
		
	}

	CherryReaderUnit::~CherryReaderUnit()
	{
		
	}

	PCSCReaderUnitType CherryReaderUnit::getPCSCType() const
	{
		return PCSC_RUT_CHERRY;
	}
}
