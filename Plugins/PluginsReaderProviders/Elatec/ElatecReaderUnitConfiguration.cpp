/**
 * \file ElatecReaderUnitConfiguration.cpp
 * \author Maxime CHAMLEY <maxime.chamley@islog.eu>
 * \brief  Elatec reader unit configuration.
 */

#include "ElatecReaderUnitConfiguration.h"


namespace LOGICALACCESS
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
