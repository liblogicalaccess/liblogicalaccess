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
		: ISO7816ReaderUnitConfiguration(READER_PCSC)
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

	unsigned int PCSCReaderUnitConfiguration::getTransmissionProtocol() const
	{
		return d_protocol;
	}

	void PCSCReaderUnitConfiguration::setTransmissionProtocol(unsigned int protocol)
	{
		d_protocol = protocol;
	}

	PCSCShareMode PCSCReaderUnitConfiguration::getShareMode() const
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
		ISO7816ReaderUnitConfiguration::serialize(node);

		node.put("TransmissionProtocol", d_protocol);
		node.put("ShareMode", d_share_mode);

		parentNode.add_child(PCSCReaderUnitConfiguration::getDefaultXmlNodeName(), node);
	}

	void PCSCReaderUnitConfiguration::unSerialize(boost::property_tree::ptree& node)
	{
		ISO7816ReaderUnitConfiguration::unSerialize(node);

		d_protocol = node.get_child("TransmissionProtocol").get_value<unsigned int>();
		d_share_mode = static_cast<PCSCShareMode>(node.get_child("ShareMode").get_value<unsigned int>());
	}

	std::string PCSCReaderUnitConfiguration::getDefaultXmlNodeName() const
	{
		return "PCSCReaderUnitConfiguration";
	}

}
