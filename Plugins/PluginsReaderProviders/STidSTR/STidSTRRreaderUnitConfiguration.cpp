/**
 * \file STidSTRReaderUnitConfiguration.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief STidSTR reader unit configuration.
 */

#include "STidSTRReaderUnitConfiguration.h"

namespace LOGICALACCESS
{
	STidSTRReaderUnitConfiguration::STidSTRReaderUnitConfiguration()
		: ReaderUnitConfiguration("STidSTR")
	{
		//INFO_("Constructor");
		resetConfiguration();
	}

	STidSTRReaderUnitConfiguration::~STidSTRReaderUnitConfiguration()
	{
		//INFO_("Destructor");
	}

	void STidSTRReaderUnitConfiguration::resetConfiguration()
	{
		d_rs485Address = 0;
		d_communicationType = STID_RS232;
		d_communicationMode = STID_CM_PLAIN;
		d_key_hmac.reset(new HMAC1Key(""));
		d_key_aes.reset(new AES128Key(""));
	}

	void STidSTRReaderUnitConfiguration::serialize(boost::property_tree::ptree& parentNode)
	{
		boost::property_tree::ptree node;

		node.put("RS485Address", d_rs485Address);
		node.put("CommunicationType", d_communicationType);
		node.put("CommunicationMode", d_communicationMode);
		d_key_hmac->serialize(node);
		d_key_aes->serialize(node);

		parentNode.add_child(getDefaultXmlNodeName(), node);
	}

	void STidSTRReaderUnitConfiguration::unSerialize(boost::property_tree::ptree& node)
	{
		INFO_SIMPLE_("Unserializing reader unit configuration...");

		d_rs485Address = node.get_child("RS485Address").get_value<unsigned char>();
		d_communicationType = static_cast<STidCommunicationType>(node.get_child("CommunicationType").get_value<unsigned int>());
		d_communicationMode = static_cast<STidCommunicationMode>(node.get_child("CommunicationMode").get_value<unsigned int>());
		d_key_hmac->unSerialize(node.get_child(d_key_hmac->getDefaultXmlNodeName()));
		d_key_aes->unSerialize(node.get_child(d_key_aes->getDefaultXmlNodeName()));
	}

	std::string STidSTRReaderUnitConfiguration::getDefaultXmlNodeName() const
	{
		return "STidSTRReaderUnitConfiguration";
	}

	unsigned char STidSTRReaderUnitConfiguration::getRS485Address() const
	{
		INFO_("RS485 Address {0x%x(%d)}", d_rs485Address, d_rs485Address);
		return d_rs485Address;
	}

	void STidSTRReaderUnitConfiguration::setRS485Address(unsigned char address)
	{
		INFO_("RS485 Address {0x%x(%d)}", address, address);
		d_rs485Address = address;
	}

	STidCommunicationType STidSTRReaderUnitConfiguration::getCommunicationType() const
	{
		switch (d_communicationType)
		{
			case STID_RS232: INFO_("Get Communication type {0x%x(%d)} {STID_RS232}", d_communicationType, d_communicationType); break;
			case STID_RS485: INFO_("Get Communication type {0x%x(%d)} {STID_RS485}", d_communicationType, d_communicationType); break;
		}
		return d_communicationType;
	}

	void STidSTRReaderUnitConfiguration::setCommunicationType(STidCommunicationType ctype)
	{
		d_communicationType = ctype;
		switch (d_communicationType)
		{
			case STID_RS232: INFO_("Set Communication type {0x%x(%d)} {STID_RS232}", d_communicationType, d_communicationType); break;
			case STID_RS485: INFO_("Set Communication type {0x%x(%d)} {STID_RS485}", d_communicationType, d_communicationType); break;
		}
	}

	STidCommunicationMode STidSTRReaderUnitConfiguration::getCommunicationMode() const
	{
		switch (d_communicationMode)
		{
			case STID_CM_PLAIN: INFO_("Get Communication mode {0x%x(%d)} {STID_CM_PLAIN}", d_communicationMode, d_communicationMode); break;
			case STID_CM_SIGNED: INFO_("Get Communication mode {0x%x(%d)} {STID_CM_SIGNED}", d_communicationMode, d_communicationMode); break;
			case STID_CM_CIPHERED: INFO_("Get Communication mode {0x%x(%d)} {STID_CM_CIPHERED}", d_communicationMode, d_communicationMode); break;
			case STID_CM_CIPHERED_AND_SIGNED: INFO_("Get Communication mode {0x%x(%d)} {STID_CM_CIPHERED_AND_SIGNED}", d_communicationMode, d_communicationMode); break;
			case STID_CM_RESERVED: INFO_("Get Communication mode {0x%x(%d)} {STID_CM_RESERVED}", d_communicationMode, d_communicationMode); break;
		}
		return d_communicationMode;
	}

	void STidSTRReaderUnitConfiguration::setCommunicationMode(STidCommunicationMode cmode)
	{
		d_communicationMode = cmode;
		switch (d_communicationMode)
		{
			case STID_CM_PLAIN: INFO_("Set Communication mode {0x%x(%d)} {STID_CM_PLAIN}", d_communicationMode, d_communicationMode); break;
			case STID_CM_SIGNED: INFO_("Set Communication mode {0x%x(%d)} {STID_CM_SIGNED}", d_communicationMode, d_communicationMode); break;
			case STID_CM_CIPHERED: INFO_("Set Communication mode {0x%x(%d)} {STID_CM_CIPHERED}", d_communicationMode, d_communicationMode); break;
			case STID_CM_CIPHERED_AND_SIGNED: INFO_("Set Communication mode {0x%x(%d)} {STID_CM_CIPHERED_AND_SIGNED}", d_communicationMode, d_communicationMode); break;
			case STID_CM_RESERVED: INFO_("Set Communication mode {0x%x(%d)} {STID_CM_RESERVED}", d_communicationMode, d_communicationMode); break;
		}
	}

	boost::shared_ptr<HMAC1Key> STidSTRReaderUnitConfiguration::getHMACKey() const
	{
		DEBUG_("Get HMAC key data {%s}", d_key_hmac->toString().c_str());
		return d_key_hmac;
	}

	void STidSTRReaderUnitConfiguration::setHMACKey(boost::shared_ptr<HMAC1Key> key)
	{
		if (key)
		{
			DEBUG_("Set HMAC key data {%s}", key->toString().c_str());
			d_key_hmac = key;
		}
	}

	boost::shared_ptr<AES128Key> STidSTRReaderUnitConfiguration::getAESKey() const
	{
		DEBUG_("Get AES 128 key data {%s}", d_key_aes->toString().c_str());
		return d_key_aes;
	}

	void STidSTRReaderUnitConfiguration::setAESKey(boost::shared_ptr<AES128Key> key)
	{
		if (key)
		{
			DEBUG_("Set AES 128 key data {%s}", key->toString().c_str());
			d_key_aes = key;
		}
	}
}
