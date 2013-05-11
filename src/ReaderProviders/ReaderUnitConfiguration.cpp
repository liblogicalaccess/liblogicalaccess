/**
 * \file ReaderUnitConfiguration.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Reader unit configuration.
 */

#include "logicalaccess/ReaderProviders/ReaderUnitConfiguration.h"

#include <iostream>
#include <iomanip>
#include <sstream>


namespace logicalaccess
{
	ReaderUnitConfiguration::ReaderUnitConfiguration(std::string rpt)
		: XmlSerializable(), d_readerProviderType(rpt)
	{
	}

	ReaderUnitConfiguration::~ReaderUnitConfiguration()
	{
	}

	std::string ReaderUnitConfiguration::getRPType() const
	{
		return d_readerProviderType;
	}
}