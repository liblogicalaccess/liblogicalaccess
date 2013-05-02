/**
 * \file ProxLocation.cpp
 * \author Maxime CHAMLEY <maxime.chamley@islog.eu>
 * \brief Location informations for Prox card.
 */

#include "ProxLocation.h"

namespace LOGICALACCESS
{
	ProxLocation::ProxLocation()
	{
		bit = 0;
	}

	ProxLocation::~ProxLocation()
	{		
	}

	std::vector<unsigned char> ProxLocation::getLinearData() const
	{
		std::vector<unsigned char> data;

		data.push_back(static_cast<unsigned char>(bit));

		return data;
	}

	void ProxLocation::setLinearData(const std::vector<unsigned char>& data, size_t offset)
	{
		bit = static_cast<int>(data[offset++]);
	}

	size_t ProxLocation::getDataSize()
	{
		return (1);
	}

	void ProxLocation::serialize(boost::property_tree::ptree& parentNode)
	{
		boost::property_tree::ptree node;

		node.put("Bit", bit);

		parentNode.add_child(getDefaultXmlNodeName(), node);
	}

	void ProxLocation::unSerialize(boost::property_tree::ptree& node)
	{
		bit = node.get_child("Bit").get_value<int>();
	}

	std::string ProxLocation::getDefaultXmlNodeName() const
	{
		return "ProxLocation";
	}

	bool ProxLocation::operator==(const Location& location) const
	{
		if (!Location::operator==(location))
			return false;

		const ProxLocation* pxLocation = dynamic_cast<const ProxLocation*>(&location);

		return (bit == pxLocation->bit
			);
	}
}

