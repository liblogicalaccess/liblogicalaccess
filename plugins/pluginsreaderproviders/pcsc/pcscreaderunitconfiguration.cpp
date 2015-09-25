/**
 * \file pcscreaderunitconfiguration.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief PC/SC reader unit configuration.
 */

#include <boost/property_tree/ptree.hpp>
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
        d_share_mode = SC_SHARED;
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

	PCSCReaderUnitType PCSCReaderUnitConfiguration::getPCSCType() const
	{
		return PCSC_RUT_DEFAULT;
	}

	std::string pcsc_share_mode_to_string(PCSCShareMode sm)
	{
		switch (sm)
		{
		case SC_SHARED:
			return "SHARED";
		case SC_SHARED_EXCLUSIVE:
			return "EXCLUSIVE";
		case SC_DIRECT:
			return "DIRECT";
		}
		return "UNKOWN";
	}

	std::string pcsc_protocol_to_string(unsigned long proto)
	{
		if (proto == SCARD_PROTOCOL_T0)
			return "T=0";
		if (proto == SCARD_PROTOCOL_T1)
			return "T=1";
		if (proto == SCARD_PROTOCOL_RAW)
			return "RAW";
		if (proto == SCARD_PROTOCOL_UNDEFINED)
			return "UNDEFINED";
		return "UNKOWN";
	}
}
