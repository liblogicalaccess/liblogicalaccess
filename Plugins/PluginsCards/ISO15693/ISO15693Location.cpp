/**
 * \file ISO15693Location.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Location informations for ISO15693 card.
 */

#include "ISO15693Location.h"

namespace logicalaccess
{
	ISO15693Location::ISO15693Location()
	{
		block = 0;
	}

	ISO15693Location::~ISO15693Location()
	{		
	}

	std::vector<unsigned char> ISO15693Location::getLinearData() const
	{
		std::vector<unsigned char> data;

		data.push_back(static_cast<unsigned char>(block));

		return data;
	}

	void ISO15693Location::setLinearData(const std::vector<unsigned char>& data, size_t offset)
	{
		block = static_cast<int>(data[offset++]);
	}

	size_t ISO15693Location::getDataSize()
	{
		return (1);
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

		const ISO15693Location* pxLocation = dynamic_cast<const ISO15693Location*>(&location);

		return (block == pxLocation->block
			);
	}
}

