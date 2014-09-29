/**
 * \file iso15693location.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Location informations for ISO15693 card.
 */

#include "iso15693location.hpp"

namespace logicalaccess
{
	ISO15693Location::ISO15693Location()
	{
		block = 0;
	}

	ISO15693Location::~ISO15693Location()
	{		
	}

	void ISO15693Location::serialize(boost::property_tree::ptree& parentNode)
	{
		boost::property_tree::ptree node;
		
		node.put("Block", block);

		parentNode.add_child(getDefaultXmlNodeName(), node);
	}

	void ISO15693Location::unSerialize(boost::property_tree::ptree& node)
	{
		block = node.get_child("Block").get_value<int>();
	}

	std::string ISO15693Location::getDefaultXmlNodeName() const
	{
		return "ISO15693Location";
	}

	bool ISO15693Location::operator==(const Location& location) const
	{
		if (!Location::operator==(location))
			return false;

		if (typeid(location) != typeid(ISO15693Location))
			return false;

		const ISO15693Location* pxLocation = dynamic_cast<const ISO15693Location*>(&location);

		if (!pxLocation)
			return false;

		return (block == pxLocation->block);
	}
}

