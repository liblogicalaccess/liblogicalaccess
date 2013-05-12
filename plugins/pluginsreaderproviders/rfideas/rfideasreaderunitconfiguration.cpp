/**
 * \file rfideasreaderunitconfiguration.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief RFIDeas reader unit configuration.
 */

#include "rfideasreaderunitconfiguration.hpp"
#include "rfideasreaderprovider.hpp"

namespace logicalaccess
{
	RFIDeasReaderUnitConfiguration::RFIDeasReaderUnitConfiguration()
		: ReaderUnitConfiguration(READER_RFIDEAS)
	{
		resetConfiguration();
	}

	RFIDeasReaderUnitConfiguration::~RFIDeasReaderUnitConfiguration()
	{
	}

	void RFIDeasReaderUnitConfiguration::resetConfiguration()
	{
	}

	void RFIDeasReaderUnitConfiguration::serialize(boost::property_tree::ptree& parentNode)
	{
		boost::property_tree::ptree node;
		parentNode.add_child(getDefaultXmlNodeName(), node);
	}

	void RFIDeasReaderUnitConfiguration::unSerialize(boost::property_tree::ptree& /*node*/)
	{
		//TODO: Implement this.
	}

	std::string RFIDeasReaderUnitConfiguration::getDefaultXmlNodeName() const
	{
		return "RFIDeasReaderUnitConfiguration";
	}
}
