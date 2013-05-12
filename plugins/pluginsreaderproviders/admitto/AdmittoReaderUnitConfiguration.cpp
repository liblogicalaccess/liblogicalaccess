/**
 * \file AdmittoReaderUnitConfiguration.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief  Admitto reader unit configuration.
 */

#include "AdmittoReaderUnitConfiguration.h"



namespace logicalaccess
{
	AdmittoReaderUnitConfiguration::AdmittoReaderUnitConfiguration()
		: ReaderUnitConfiguration("Admitto")
	{
		resetConfiguration();
	}

	AdmittoReaderUnitConfiguration::~AdmittoReaderUnitConfiguration()
	{
	}

	void AdmittoReaderUnitConfiguration::resetConfiguration()
	{
	}

	void AdmittoReaderUnitConfiguration::serialize(boost::property_tree::ptree& parentNode)
	{
		boost::property_tree::ptree node;
		parentNode.add_child(getDefaultXmlNodeName(), node);
	}

	void AdmittoReaderUnitConfiguration::unSerialize(boost::property_tree::ptree& /*node*/)
	{
		INFO_SIMPLE_("Unserializing reader unit configuration...");
	}

	std::string AdmittoReaderUnitConfiguration::getDefaultXmlNodeName() const
	{
		return "AdmittoReaderUnitConfiguration";
	}
}
