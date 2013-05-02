/**
 * \file AxessTMC13ReaderUnitConfiguration.cpp
 * \author Maxime CHAMLEY <maxime.chamley@islog.eu>
 * \brief  AxessTMC 13Mhz reader unit configuration.
 */

#include "AxessTMC13ReaderUnitConfiguration.h"


namespace LOGICALACCESS
{
	AxessTMC13ReaderUnitConfiguration::AxessTMC13ReaderUnitConfiguration()
		: ReaderUnitConfiguration("AxessTMC13")
	{
		resetConfiguration();
	}

	AxessTMC13ReaderUnitConfiguration::~AxessTMC13ReaderUnitConfiguration()
	{
	}

	void AxessTMC13ReaderUnitConfiguration::resetConfiguration()
	{
	}

	void AxessTMC13ReaderUnitConfiguration::serialize(boost::property_tree::ptree& parentNode)
	{
		boost::property_tree::ptree node;
		parentNode.add_child(getDefaultXmlNodeName(), node);
	}

	void AxessTMC13ReaderUnitConfiguration::unSerialize(boost::property_tree::ptree& /*node*/)
	{
		//TODO: Implement this.
	}

	std::string AxessTMC13ReaderUnitConfiguration::getDefaultXmlNodeName() const
	{
		return "AxessTMC13ReaderUnitConfiguration";
	}
}
