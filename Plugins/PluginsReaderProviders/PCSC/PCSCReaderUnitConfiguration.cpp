/**
 * \file PCSCReaderUnitConfiguration.cpp
 * \author Maxime CHAMLEY <maxime.chamley@islog.eu>
 * \brief PC/SC reader unit configuration.
 */

#include "PCSCReaderUnitConfiguration.h"


namespace LOGICALACCESS
{
	PCSCReaderUnitConfiguration::PCSCReaderUnitConfiguration()
		: ReaderUnitConfiguration("PCSC")
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

		parentNode.add_child(PCSCReaderUnitConfiguration::getDefaultXmlNodeName(), node);
	}

	void PCSCReaderUnitConfiguration::unSerialize(boost::property_tree::ptree& node)
	{
		d_protocol = node.get_child("TransmissionProtocol").get_value<unsigned int>();
		d_share_mode = static_cast<PCSCShareMode>(node.get_child("ShareMode").get_value<unsigned int>());
	}

	std::string PCSCReaderUnitConfiguration::getDefaultXmlNodeName() const
	{
		return "PCSCReaderUnitConfiguration";
	}
}
