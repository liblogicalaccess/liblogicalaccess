/**
 * \file mifareultralightlocation.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Location informations for Mifare Ultralight card.
 */

#include "mifareultralightlocation.hpp"

namespace logicalaccess
{
	MifareUltralightLocation::MifareUltralightLocation() :
		page(4),
		byte(0)
	{
	}

	MifareUltralightLocation::~MifareUltralightLocation()
	{		
	}

	void MifareUltralightLocation::serialize(boost::property_tree::ptree& parentNode)
	{
		boost::property_tree::ptree node;

		node.put("Page", page);
		node.put("Byte", byte);

		parentNode.add_child(getDefaultXmlNodeName(), node);
	}

	void MifareUltralightLocation::unSerialize(boost::property_tree::ptree& node)
	{
		page = node.get_child("Page").get_value<int>();
		byte = node.get_child("Byte").get_value<int>();
	}

	std::string MifareUltralightLocation::getDefaultXmlNodeName() const
	{
		return "MifareUltralightLocation";
	}

	bool MifareUltralightLocation::operator==(const Location& location) const
	{
		if (!Location::operator==(location))
			return false;

		if (typeid(location) != typeid(MifareUltralightLocation))
			return false;

		const MifareUltralightLocation* mLocation = dynamic_cast<const MifareUltralightLocation*>(&location);

		return (page == mLocation->page
				&& byte == mLocation->byte);
	}
}

