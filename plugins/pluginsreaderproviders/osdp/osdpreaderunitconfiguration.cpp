/**
 * \file STidSTRReaderUnitConfiguration.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief STidSTR reader unit configuration.
 */

#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include "osdpreaderunitconfiguration.hpp"
#include "osdpreaderprovider.hpp"
#include "logicalaccess/logs.hpp"

namespace logicalaccess
{
	OSDPReaderUnitConfiguration::OSDPReaderUnitConfiguration()
		: ReaderUnitConfiguration(READER_OSDP)
	{
		resetConfiguration();
	}

	OSDPReaderUnitConfiguration::~OSDPReaderUnitConfiguration()
	{
		
	}

	void OSDPReaderUnitConfiguration::resetConfiguration()
	{
		d_rs485Address = 0;
		d_master_key_aes.reset(new AES128Key(""));
		d_scbk_d_key_aes.reset(new AES128Key(""));
		d_scbk_key_aes.reset(new AES128Key(""));
	}

	void OSDPReaderUnitConfiguration::serialize(boost::property_tree::ptree& parentNode)
	{
		boost::property_tree::ptree node;

		node.put("RS485Address", d_rs485Address);
		d_scbk_d_key_aes->serialize(node);
		d_scbk_key_aes->serialize(node);
		d_master_key_aes->serialize(node);
		parentNode.add_child(getDefaultXmlNodeName(), node);
	}

	void OSDPReaderUnitConfiguration::unSerialize(boost::property_tree::ptree& node)
	{
		LOG(LogLevel::INFOS) << "Unserializing reader unit configuration...";

		d_rs485Address = node.get_child("RS485Address").get_value<unsigned char>();
		d_scbk_d_key_aes->unSerialize(node.get_child(d_scbk_d_key_aes->getDefaultXmlNodeName()));
		d_scbk_key_aes->unSerialize(node.get_child(d_scbk_key_aes->getDefaultXmlNodeName()));
		d_master_key_aes->unSerialize(node.get_child(d_master_key_aes->getDefaultXmlNodeName()));
	}

	std::string OSDPReaderUnitConfiguration::getDefaultXmlNodeName() const
	{
		return "OSDPReaderUnitConfiguration";
	}

	unsigned char OSDPReaderUnitConfiguration::getRS485Address() const
	{
		LOG(LogLevel::INFOS) << "RS485 Address {0x" << std::hex <<d_rs485Address << std::dec << "(" << d_rs485Address << ")}";
		return d_rs485Address;
	}

	void OSDPReaderUnitConfiguration::setRS485Address(unsigned char address)
	{
		LOG(LogLevel::INFOS) << "RS485 Address {0x" << std::hex << address << std::dec << "(" << address << ")}";
		d_rs485Address = address;
	}

	std::shared_ptr<AES128Key> OSDPReaderUnitConfiguration::getMasterKey() const
	{
		return d_master_key_aes;
	}

	void OSDPReaderUnitConfiguration::setMasterKey(std::shared_ptr<AES128Key> key)
	{
		d_master_key_aes = key;
	}

	std::shared_ptr<AES128Key> OSDPReaderUnitConfiguration::getSCBKKey() const
	{
		return d_scbk_key_aes;
	}

	void OSDPReaderUnitConfiguration::setSCBKKey(std::shared_ptr<AES128Key> key)
	{
		d_scbk_key_aes = key;
	}

	std::shared_ptr<AES128Key> OSDPReaderUnitConfiguration::getSCBKDKey() const
	{
		return d_scbk_d_key_aes;
	}

	void OSDPReaderUnitConfiguration::setSCBKDKey(std::shared_ptr<AES128Key> key)
	{
		d_scbk_d_key_aes = key;
	}
}
