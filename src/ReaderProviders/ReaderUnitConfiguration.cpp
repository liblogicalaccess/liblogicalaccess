/**
 * \file ReaderUnitConfiguration.cpp
 * \author Maxime CHAMLEY <maxime.chamley@islog.eu>
 * \brief Reader unit configuration.
 */

#include "logicalaccess/ReaderProviders/ReaderUnitConfiguration.h"

#include <iostream>
#include <iomanip>
#include <sstream>


namespace LOGICALACCESS
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