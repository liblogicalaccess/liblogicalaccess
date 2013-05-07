/**
 * \file SCMReaderUnit.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief SCM reader unit.
 */

#include "../Readers/SCMReaderUnit.h"

#include <iostream>
#include <iomanip>
#include <sstream>


namespace LOGICALACCESS
{
	SCMReaderUnit::SCMReaderUnit(const std::string& name)
		: PCSCReaderUnit(name)
	{
		
	}

	SCMReaderUnit::~SCMReaderUnit()
	{
		
	}

	PCSCReaderUnitType SCMReaderUnit::getPCSCType() const
	{
		return PCSC_RUT_SCM_SDI010;
	}
}
