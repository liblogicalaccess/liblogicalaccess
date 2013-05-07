/**
 * \file PromagReaderUnitConfiguration.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief  Promag reader unit configuration.
 */

#include "PromagReaderUnitConfiguration.h"


namespace LOGICALACCESS
{
	PromagReaderUnitConfiguration::PromagReaderUnitConfiguration()
		: ReaderUnitConfiguration("Promag")
	{
		resetConfiguration();
	}

	PromagReaderUnitConfiguration::~PromagReaderUnitConfiguration()
	{
	}

	void PromagReaderUnitConfiguration::resetConfiguration()
	{
	}

	void PromagReaderUnitConfiguration::serialize(boost::property_tree::ptree& parentNode)
	{
		boost::property_tree::ptree node;
		parentNode.add_child(getDefaultXmlNodeName(), node);
	}

	void PromagReaderUnitConfiguration::unSerialize(boost::property_tree::ptree& /*node*/)
	{
		//TODO: Implement this.
	}

	std::string PromagReaderUnitConfiguration::getDefaultXmlNodeName() const
	{
		return "PromagReaderUnitConfiguration";
	}
}
