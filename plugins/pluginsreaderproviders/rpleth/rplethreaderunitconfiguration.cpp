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
		d_length = 16;
	}

	void RplethReaderUnitConfiguration::setConfiguration(const std::string& readerAddress, int port)
	{
		d_readerAddress = readerAddress;
		d_port = port;
	}

	void RplethReaderUnitConfiguration::setWiegandConfiguration (unsigned char offset, unsigned char length)
	{
		d_offset = offset;
		d_length = length;
	}

	void RplethReaderUnitConfiguration::serialize(boost::property_tree::ptree& parentNode)
	{
		boost::property_tree::ptree node;
		node.put("ReaderAddress", d_readerAddress);
		node.put("Port", d_port);
		node.put("Offset", d_offset);
		node.put("Length", d_length);
		parentNode.add_child(getDefaultXmlNodeName(), node);
	}

	void RplethReaderUnitConfiguration::unSerialize(boost::property_tree::ptree& node)
	{
		d_readerAddress = node.get_child("ReaderAddress").get_value<std::string>();
		d_port = node.get_child("Port").get_value<int>();
		d_offset = node.get_child("Offset").get_value<unsigned char>();
		d_length = node.get_child("Length").get_value<unsigned char>();
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

	unsigned char RplethReaderUnitConfiguration::getOffset() const
	{
		return d_offset;
	}

	void RplethReaderUnitConfiguration::setOffset(unsigned char offset)
	{
		d_offset = offset;
	}
			
	unsigned char RplethReaderUnitConfiguration::getLength() const
	{
		return d_length;
	}

	void RplethReaderUnitConfiguration::setLength(unsigned char length)
	{
		d_length = length;
	}
}
