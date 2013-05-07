/**
 * \file DeisterReaderUnitConfiguration.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Deister reader unit configuration.
 */

#include "DeisterReaderUnitConfiguration.h"


namespace LOGICALACCESS
{
	DeisterReaderUnitConfiguration::DeisterReaderUnitConfiguration()
		: ReaderUnitConfiguration("Deister")
	{
		resetConfiguration();
	}

	DeisterReaderUnitConfiguration::~DeisterReaderUnitConfiguration()
	{
	}

	void DeisterReaderUnitConfiguration::resetConfiguration()
	{
	}

	void DeisterReaderUnitConfiguration::serialize(boost::property_tree::ptree& parentNode)
	{
		boost::property_tree::ptree node;
		parentNode.add_child(getDefaultXmlNodeName(), node);
	}

	void DeisterReaderUnitConfiguration::unSerialize(boost::property_tree::ptree& /*node*/)
	{
		//TODO: Implement this.
	}

	std::string DeisterReaderUnitConfiguration::getDefaultXmlNodeName() const
	{
		return "DeisterReaderUnitConfiguration";
	}
}
