/**
 * \file a3mlgm5600readerunitconfiguration.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief A3MLGM5600 reader unit configuration.
 */

#include "a3mlgm5600readerunitconfiguration.hpp"
#include "a3mlgm5600readerprovider.hpp"


namespace logicalaccess
{
	A3MLGM5600ReaderUnitConfiguration::A3MLGM5600ReaderUnitConfiguration()
		: ReaderUnitConfiguration(READER_A3MLGM5600)
	{
		resetConfiguration();
	}

	A3MLGM5600ReaderUnitConfiguration::~A3MLGM5600ReaderUnitConfiguration()
	{
	}

	void A3MLGM5600ReaderUnitConfiguration::resetConfiguration()
	{
		d_localPort = 2000;
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

		node.put("LocalPort", d_localPort);

		parentNode.add_child(getDefaultXmlNodeName(), node);
	}

	void A3MLGM5600ReaderUnitConfiguration::unSerialize(boost::property_tree::ptree& node)
	{
		d_localPort = node.get_child("LocalPort").get_value<int>();
	}

	std::string A3MLGM5600ReaderUnitConfiguration::getDefaultXmlNodeName() const
	{
		return "A3MLGM5600ReaderUnitConfiguration";
	}
}
