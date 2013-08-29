/**
 * \file omnikeyxx27readerunit.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Omnikey XX27 reader unit.
 */

#include "../readers/omnikeyxx27readerunit.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>


namespace logicalaccess
{
	OmnikeyXX27ReaderUnit::OmnikeyXX27ReaderUnit(const std::string& name)
		: OmnikeyReaderUnit(name)
	{
		
	}

	OmnikeyXX27ReaderUnit::~OmnikeyXX27ReaderUnit()
	{
		
	}	

	PCSCReaderUnitType OmnikeyXX27ReaderUnit::getPCSCType() const
	{
		return PCSC_RUT_OMNIKEY_XX27;
	}

	boost::shared_ptr<ReaderCardAdapter> OmnikeyXX27ReaderUnit::getReaderCardAdapter(std::string /*type*/)
	{
		return getDefaultReaderCardAdapter();
	}
}
