/**
 * \file mifareultralightaccessinfo.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Access informations for Mifare Ultralight card.
 */

#include "mifareultralightaccessinfo.hpp"
#include "mifareultralightchip.hpp"

namespace logicalaccess
{
	MifareUltralightAccessInfo::MifareUltralightAccessInfo()
	{
		lockPage = false;
	}

	MifareUltralightAccessInfo::~MifareUltralightAccessInfo()
	{		
	}

	std::string MifareUltralightAccessInfo::getCardType() const
	{
		return CHIP_MIFAREULTRALIGHT;
	}

	void MifareUltralightAccessInfo::generateInfos()
	{		
		
	}

	void MifareUltralightAccessInfo::serialize(boost::property_tree::ptree& parentNode)
	{
		boost::property_tree::ptree node;
		node.put("LockPage", lockPage);
		parentNode.add_child(getDefaultXmlNodeName(), node);
	}

	void MifareUltralightAccessInfo::unSerialize(boost::property_tree::ptree& node)
	{
		lockPage = node.get_child("LockPage").get_value<bool>();
	}

	std::string MifareUltralightAccessInfo::getDefaultXmlNodeName() const
	{
		return "MifareUltralightAccessInfo";
	}

	bool MifareUltralightAccessInfo::operator==(const AccessInfo& ai) const
	{
		if (!AccessInfo::operator==(ai))
			return false;

		const MifareUltralightAccessInfo* mAi = dynamic_cast<const MifareUltralightAccessInfo*>(&ai);

		return (lockPage == mAi->lockPage);
	}
}

