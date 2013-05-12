/**
 * \file ElatecReaderUnitConfiguration.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief  Elatec reader unit configuration.
 */

#include "ElatecReaderUnitConfiguration.h"


namespace logicalaccess
{
	ElatecReaderUnitConfiguration::ElatecReaderUnitConfiguration()
		: ReaderUnitConfiguration("Elatec")
	{
		resetConfiguration();
	}

	ElatecReaderUnitConfiguration::~ElatecReaderUnitConfiguration()
	{
	}

	void ElatecReaderUnitConfiguration::resetConfiguration()
	{
	}

	void ElatecReaderUnitConfiguration::serialize(boost::property_tree::ptree& parentNode)
	{
		boost::property_tree::ptree node;
		parentNode.add_child(getDefaultXmlNodeName(), node);
	}

	void ElatecReaderUnitConfiguration::unSerialize(boost::property_tree::ptree& /*node*/)
	{
		//TODO: Implement this.
	}

	std::string ElatecReaderUnitConfiguration::getDefaultXmlNodeName() const
	{
		return "ElatecReaderUnitConfiguration";
	}
}
