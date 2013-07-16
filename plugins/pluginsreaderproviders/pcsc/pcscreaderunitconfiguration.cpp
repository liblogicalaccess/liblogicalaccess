/**
 * \file pcscreaderunitconfiguration.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief PC/SC reader unit configuration.
 */

#include "pcscreaderunitconfiguration.hpp"
#include "pcscreaderprovider.hpp"
#include "pcscreaderunit.hpp"

namespace logicalaccess
{
	PCSCReaderUnitConfiguration::PCSCReaderUnitConfiguration()
		: ReaderUnitConfiguration(READER_PCSC)
	{
		resetConfiguration();
	}

	PCSCReaderUnitConfiguration::~PCSCReaderUnitConfiguration()
	{
	}

	void PCSCReaderUnitConfiguration::resetConfiguration()
	{
		d_protocol = SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1;
		d_share_mode = SM_SHARED;
		d_sam_type = SAM_NONE;
		d_sam_reader_name = "";
	}

	unsigned int PCSCReaderUnitConfiguration::getTransmissionProtocol()
	{
		return d_protocol;
	}

	void PCSCReaderUnitConfiguration::setTransmissionProtocol(unsigned int protocol)
	{
		d_protocol = protocol;
	}

	PCSCShareMode PCSCReaderUnitConfiguration::getShareMode()
	{
		return d_share_mode;
	}

	void PCSCReaderUnitConfiguration::setShareMode(PCSCShareMode share_mode)
	{
		d_share_mode = share_mode;
	}

	void PCSCReaderUnitConfiguration::serialize(boost::property_tree::ptree& parentNode)
	{
		boost::property_tree::ptree node;
		
		node.put("TransmissionProtocol", d_protocol);
		node.put("ShareMode", d_share_mode);
		node.put("SAMType", d_sam_type);
		node.put("SAMReaderName", d_sam_reader_name);

		parentNode.add_child(PCSCReaderUnitConfiguration::getDefaultXmlNodeName(), node);
	}

	void PCSCReaderUnitConfiguration::unSerialize(boost::property_tree::ptree& node)
	{
		d_protocol = node.get_child("TransmissionProtocol").get_value<unsigned int>();
		d_share_mode = static_cast<PCSCShareMode>(node.get_child("ShareMode").get_value<unsigned int>());
		d_sam_type = static_cast<SAMType>(node.get_child("SAMType").get_value<unsigned int>());
		d_sam_reader_name = node.get_child("SAMReaderName").get_value<std::string>();
	}

	std::string PCSCReaderUnitConfiguration::getDefaultXmlNodeName() const
	{
		return "PCSCReaderUnitConfiguration";
	}

	void PCSCReaderUnitConfiguration::setSAMType(SAMType t)
	{
		d_sam_type = t;
	}

	SAMType PCSCReaderUnitConfiguration::getSAMType()
	{
		return d_sam_type;
	}


	void PCSCReaderUnitConfiguration::setSAMReaderName(std::string t)
	{
		d_sam_reader_name = t;
	}

	std::string PCSCReaderUnitConfiguration::getSAMReaderName()
	{
		return d_sam_reader_name;
	}
}
