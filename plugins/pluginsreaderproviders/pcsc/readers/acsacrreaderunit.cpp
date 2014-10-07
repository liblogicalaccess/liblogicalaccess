/**
 * \file acsacrreaderunit.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief ACS ACR reader unit.
 */

#include "../readers/acsacrreaderunit.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>


namespace logicalaccess
{
	ACSACRReaderUnit::ACSACRReaderUnit(const std::string& name)
		: PCSCReaderUnit(name)
	{
		
	}

	ACSACRReaderUnit::~ACSACRReaderUnit()
	{
		
	}

	PCSCReaderUnitType ACSACRReaderUnit::getPCSCType() const
	{
		return PCSC_RUT_ACS_ACR;
	}
}
