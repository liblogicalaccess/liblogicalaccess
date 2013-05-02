/**
 * \file TwicLocation.cpp
 * \author Maxime CHAMLEY <maxime.chamley@islog.eu>
 * \brief Location informations for Twic card.
 */

#include "TwicLocation.h"

namespace LOGICALACCESS
{
	TwicLocation::TwicLocation()
	{
		dataObject = 0;
	}

	TwicLocation::~TwicLocation()
	{		
	}

	std::vector<unsigned char> TwicLocation::getLinearData() const
	{
		std::vector<unsigned char> data;

		BufferHelper::setUInt64(data, dataObject);

		return data;
	}

	void TwicLocation::setLinearData(const std::vector<unsigned char>& data, size_t offset)
	{
		dataObject = BufferHelper::getUInt64(data, offset);
	}

	size_t TwicLocation::getDataSize()
	{
		return (sizeof(dataObject));
	}

	void TwicLocation::serialize(boost::property_tree::ptree& parentNode)
	{
		boost::property_tree::ptree node;

		node.put("DataObject", dataObject);
		node.put("Tag", tag);

		parentNode.add_child(getDefaultXmlNodeName(), node);
	}

	void TwicLocation::unSerialize(boost::property_tree::ptree& node)
	{
		dataObject = node.get_child("DataObject").get_value<uint64_t>();
	}

	std::string TwicLocation::getDefaultXmlNodeName() const
	{
		return "TwicLocation";
	}

	bool TwicLocation::operator==(const Location& location) const
	{
		if (!Location::operator==(location))
			return false;

		const TwicLocation* pxLocation = dynamic_cast<const TwicLocation*>(&location);

		return (dataObject == pxLocation->dataObject
			);
	}
}

