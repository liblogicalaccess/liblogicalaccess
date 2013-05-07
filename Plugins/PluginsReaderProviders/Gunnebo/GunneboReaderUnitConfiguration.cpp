/**
 * \file GunneboReaderUnitConfiguration.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief  Gunnebo reader unit configuration.
 */

#include "GunneboReaderUnitConfiguration.h"



namespace LOGICALACCESS
{
	GunneboReaderUnitConfiguration::GunneboReaderUnitConfiguration()
		: ReaderUnitConfiguration("Gunnebo")
	{
		resetConfiguration();
	}

	GunneboReaderUnitConfiguration::~GunneboReaderUnitConfiguration()
	{
	}

	void GunneboReaderUnitConfiguration::resetConfiguration()
	{
	}

	void GunneboReaderUnitConfiguration::serialize(boost::property_tree::ptree& parentNode)
	{
		boost::property_tree::ptree node;
		parentNode.add_child(getDefaultXmlNodeName(), node);
	}

	void GunneboReaderUnitConfiguration::unSerialize(boost::property_tree::ptree& /*node*/)
	{
		INFO_("Unserializing reader unit configuration...");
	}

	std::string GunneboReaderUnitConfiguration::getDefaultXmlNodeName() const
	{
		return "GunneboReaderUnitConfiguration";
	}
}
