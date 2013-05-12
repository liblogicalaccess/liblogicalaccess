/**
 * \file A3MLGM5600ReaderUnitConfiguration.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief A3MLGM5600 reader unit configuration.
 */

#include "A3MLGM5600ReaderUnitConfiguration.h"


namespace logicalaccess
{
	A3MLGM5600ReaderUnitConfiguration::A3MLGM5600ReaderUnitConfiguration()
		: ReaderUnitConfiguration("A3MLGM5600")
	{
		resetConfiguration();
	}

	A3MLGM5600ReaderUnitConfiguration::~A3MLGM5600ReaderUnitConfiguration()
	{
	}

	void A3MLGM5600ReaderUnitConfiguration::resetConfiguration()
	{
		d_readerIpAddress = "192.168.1.100";
		d_readerPort = 2000;
		d_localPort = 2000;
	}

	std::string A3MLGM5600ReaderUnitConfiguration::getReaderIpAddress() const
	{
		return d_readerIpAddress;
	}

	void A3MLGM5600ReaderUnitConfiguration::setReaderIpAddress(std::string ipAddress)
	{
		d_readerIpAddress = ipAddress;
	}

	int A3MLGM5600ReaderUnitConfiguration::getReaderPort() const
	{
		return d_readerPort;
	}

	void A3MLGM5600ReaderUnitConfiguration::setReaderPort(int port)
	{
		d_readerPort = port;
	}

	int A3MLGM5600ReaderUnitConfiguration::getLocalPort() const
	{
		return d_localPort;
	}

	void A3MLGM5600ReaderUnitConfiguration::setLocalPort(int port)
	{
		d_localPort = port;
	}

	void A3MLGM5600ReaderUnitConfiguration::serialize(boost::property_tree::ptree& parentNode)
	{
		boost::property_tree::ptree node;

		node.put("ReaderIpAddress", d_readerIpAddress);
		node.put("ReaderPort", d_readerPort);
		node.put("LocalPort", d_localPort);

		parentNode.add_child(getDefaultXmlNodeName(), node);
	}

	void A3MLGM5600ReaderUnitConfiguration::unSerialize(boost::property_tree::ptree& node)
	{
		d_readerIpAddress = node.get_child("ReaderIpAddress").get_value<std::string>();
		d_readerPort = node.get_child("ReaderPort").get_value<int>();
		d_localPort = node.get_child("LocalPort").get_value<int>();
	}

	std::string A3MLGM5600ReaderUnitConfiguration::getDefaultXmlNodeName() const
	{
		return "A3MLGM5600ReaderUnitConfiguration";
	}
}
