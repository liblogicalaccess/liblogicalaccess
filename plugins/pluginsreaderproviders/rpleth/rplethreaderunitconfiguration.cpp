/**
 * \file rplethreaderunitconfiguration.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief  Rpleth reader unit configuration.
 */

#include "rplethreaderunitconfiguration.hpp"
#include "rplethreaderprovider.hpp"


namespace logicalaccess
{
	RplethReaderUnitConfiguration::RplethReaderUnitConfiguration()
		: ReaderUnitConfiguration(READER_RPLETH)
	{
		resetConfiguration();
	}

	RplethReaderUnitConfiguration::~RplethReaderUnitConfiguration()
	{
	}

	void RplethReaderUnitConfiguration::resetConfiguration()
	{
		d_readerAddress = "192.168.1.100";
		d_port = 2000;
		d_offset = 1;
		d_lenght = 16;
	}

	void RplethReaderUnitConfiguration::setConfiguration(const std::string& readerAddress, int port)
	{
		d_readerAddress = readerAddress;
		d_port = port;
	}

	void RplethReaderUnitConfiguration::setWiegandConfiguration (byte offset, byte lenght)
	{
		d_offset = offset;
		d_lenght = lenght;
	}

	void RplethReaderUnitConfiguration::serialize(boost::property_tree::ptree& parentNode)
	{
		boost::property_tree::ptree node;
		node.put("ReaderAddress", d_readerAddress);
		node.put("Port", d_port);
		parentNode.add_child(getDefaultXmlNodeName(), node);
	}

	void RplethReaderUnitConfiguration::unSerialize(boost::property_tree::ptree& node)
	{
		d_readerAddress = node.get_child("ReaderAddress").get_value<std::string>();
		d_port = node.get_child("Port").get_value<int>();
	}

	std::string RplethReaderUnitConfiguration::getDefaultXmlNodeName() const
	{
		return "RplethReaderUnitConfiguration";
	}

	std::string RplethReaderUnitConfiguration::getReaderAddress() const
	{
		return d_readerAddress;
	}

	void RplethReaderUnitConfiguration::setReaderAddress(const std::string& readerAddress)
	{
		d_readerAddress = readerAddress;
	}

	int RplethReaderUnitConfiguration::getPort() const
	{
		return d_port;
	}

	void RplethReaderUnitConfiguration::setPort(int port)
	{
		d_port = port;
	}

	byte RplethReaderUnitConfiguration::getOffset() const
	{
		return d_offset;
	}

	void RplethReaderUnitConfiguration::setOffset(byte offset)
	{
		d_offset = offset;
	}
			
	byte RplethReaderUnitConfiguration::getLenght() const
	{
		return d_lenght;
	}

	void RplethReaderUnitConfiguration::setLenght(byte lenght)
	{
		d_lenght = lenght;
	}
}
