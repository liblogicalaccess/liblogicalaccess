/**
 * \file OmnikeyXX25ReaderUnit.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Omnikey XX25 reader unit.
 */

#include "../Readers/OmnikeyXX25ReaderUnit.h"

#include <iostream>
#include <iomanip>
#include <sstream>


namespace LOGICALACCESS
{
	OmnikeyXX25ReaderUnit::OmnikeyXX25ReaderUnit(const std::string& name)
		: OmnikeyReaderUnit(name)
	{
		
	}

	OmnikeyXX25ReaderUnit::~OmnikeyXX25ReaderUnit()
	{
		
	}

	PCSCReaderUnitType OmnikeyXX25ReaderUnit::getPCSCType() const
	{
		return PCSC_RUT_OMNIKEY_XX25;
	}

	std::vector<unsigned char> OmnikeyXX25ReaderUnit::getCardSerialNumber()
	{
		return std::vector<unsigned char>(d_atr, d_atr + d_atrLength);
	}
}
