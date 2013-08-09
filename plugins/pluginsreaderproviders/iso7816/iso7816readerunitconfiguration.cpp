/**
 * \file pcscreaderunitconfiguration.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief PC/SC reader unit configuration.
 */

#include "iso7816ReaderUnitConfiguration.hpp"
#include "iso7816readerunit.hpp"

namespace logicalaccess
{
	ISO7816ReaderUnitConfiguration::ISO7816ReaderUnitConfiguration(std::string rpt)
		: ReaderUnitConfiguration(rpt)
	{
		resetConfiguration();
	}

	ISO7816ReaderUnitConfiguration::ISO7816ReaderUnitConfiguration()
		: ReaderUnitConfiguration(READER_ISO7816)
	{
		resetConfiguration();
	}

	ISO7816ReaderUnitConfiguration::~ISO7816ReaderUnitConfiguration()
	{
	}

	void ISO7816ReaderUnitConfiguration::resetConfiguration()
	{
		d_sam_type = "SAM_NONE";
		d_sam_reader_name = "";
	}

	void ISO7816ReaderUnitConfiguration::serialize(boost::property_tree::ptree& node)
	{
		node.put("SAMType", d_sam_type);
		node.put("SAMReaderName", d_sam_reader_name);
	}

	void ISO7816ReaderUnitConfiguration::unSerialize(boost::property_tree::ptree& node)
	{
		d_sam_type = static_cast<std::string>(node.get_child("SAMType").get_value<std::string>());
		d_sam_reader_name = node.get_child("SAMReaderName").get_value<std::string>();
	}

	std::string ISO7816ReaderUnitConfiguration::getDefaultXmlNodeName() const
	{
		return "ISO7816ReaderUnitConfiguration";
	}

	void ISO7816ReaderUnitConfiguration::setSAMType(std::string t)
	{
		d_sam_type = t;
	}

	std::string ISO7816ReaderUnitConfiguration::getSAMType()
	{
		return d_sam_type;
	}


	void ISO7816ReaderUnitConfiguration::setSAMReaderName(std::string t)
	{
		d_sam_reader_name = t;
	}

	std::string ISO7816ReaderUnitConfiguration::getSAMReaderName()
	{
		return d_sam_reader_name;
	}
}
