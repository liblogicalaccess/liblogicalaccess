/**
 * \file AxessTMCLegicReaderUnitConfiguration.cpp
 * \author Maxime CHAMLEY <maxime.chamley@islog.eu>
 * \brief  AxessTMCLegic reader unit configuration.
 */

#include "AxessTMCLegicReaderUnitConfiguration.h"


namespace LOGICALACCESS
{
	AxessTMCLegicReaderUnitConfiguration::AxessTMCLegicReaderUnitConfiguration()
		: ReaderUnitConfiguration("AxessTMCLegic")
	{
		resetConfiguration();
	}

	AxessTMCLegicReaderUnitConfiguration::~AxessTMCLegicReaderUnitConfiguration()
	{
	}

	void AxessTMCLegicReaderUnitConfiguration::resetConfiguration()
	{
	}

	void AxessTMCLegicReaderUnitConfiguration::serialize(boost::property_tree::ptree& parentNode)
	{
		boost::property_tree::ptree node;
		parentNode.add_child(getDefaultXmlNodeName(), node);
	}

	void AxessTMCLegicReaderUnitConfiguration::unSerialize(boost::property_tree::ptree& /*node*/)
	{
		//TODO: Implement this.
	}

	std::string AxessTMCLegicReaderUnitConfiguration::getDefaultXmlNodeName() const
	{
		return "AxessTMCLegicReaderUnitConfiguration";
	}
}
