/**
 * \file desfireaccessinfo.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Access informations for DESFire card.
 */

#include "desfireaccessinfo.hpp"
#include "desfireprofile.hpp"
#include "desfirechip.hpp"

namespace logicalaccess
{	
	DESFireAccessInfo::DESFireAccessInfo()
	{
		masterCardKey.reset(new DESFireKey());
		masterCardKey->fromString("");
		masterApplicationKey.reset(new DESFireKey());
		masterApplicationKey->fromString("");
		readKey = DESFireProfile::getDefaultKey(DF_KEY_DES);
		writeKey = DESFireProfile::getDefaultKey(DF_KEY_DES);
		readKeyno = 0;
		writeKeyno = 0;
	}

	DESFireAccessInfo::~DESFireAccessInfo()
	{		
	}

	std::string DESFireAccessInfo::getCardType() const
	{
		return CHIP_DESFIRE;
	}

	void DESFireAccessInfo::generateInfos()
	{
		INFO_("Generate access information");
		readKey->fromString(generateSimpleKey(0, readKey->getLength()));
		readKeyno = 2;
		writeKey->fromString(generateSimpleKey(1000, writeKey->getLength()));
		writeKeyno = 1;
		masterApplicationKey->fromString(generateSimpleKey(2000, masterApplicationKey->getLength()));
		masterCardKey->fromString(generateSimpleKey(3000, masterCardKey->getLength()));
	}

	void DESFireAccessInfo::generateInfos(const string& csn)
	{
	        INFO_("Generate access information with CSN '%s' as seed", csn.c_str());
		long lcsn = atol(csn.c_str());

		readKey->fromString(generateSimpleKey(lcsn, readKey->getLength()));
		readKeyno = 2;
		writeKey->fromString(generateSimpleKey(lcsn + 1000, writeKey->getLength()));
		writeKeyno = 1;
		masterApplicationKey->fromString(generateSimpleKey(lcsn + 2000, masterApplicationKey->getLength()));
		masterCardKey->fromString(generateSimpleKey(lcsn + 3000, masterCardKey->getLength()));
	}

	void DESFireAccessInfo::serialize(boost::property_tree::ptree& parentNode)
	{
		boost::property_tree::ptree node;

		boost::property_tree::ptree rknode;
		readKey->serialize(rknode);
		node.add_child("ReadKey", rknode);
		node.put("ReadKeyno", readKeyno);

		boost::property_tree::ptree wknode;
		writeKey->serialize(wknode);
		node.add_child("WriteKey", wknode);
		node.put("WriteKeyno", writeKeyno);

		boost::property_tree::ptree maknode;
		masterApplicationKey->serialize(maknode);
		node.add_child("MasterApplicationKey", maknode);
		boost::property_tree::ptree mcknode;
		masterCardKey->serialize(mcknode);
		node.add_child("MasterCardKey", mcknode);

		parentNode.add_child(getDefaultXmlNodeName(), node);
	}

	void DESFireAccessInfo::unSerialize(boost::property_tree::ptree& node)
	{
		INFO_("Unserializing access information...");

		boost::dynamic_pointer_cast<XmlSerializable>(readKey)->unSerialize(node.get_child("ReadKey"), "");
		readKeyno = node.get_child("ReadKeyno").get_value<unsigned char>();
		boost::dynamic_pointer_cast<XmlSerializable>(writeKey)->unSerialize(node.get_child("WriteKey"), "");
		writeKeyno = node.get_child("WriteKeyno").get_value<unsigned char>();
		boost::dynamic_pointer_cast<XmlSerializable>(masterApplicationKey)->unSerialize(node.get_child("MasterApplicationKey"), "");
		boost::dynamic_pointer_cast<XmlSerializable>(masterCardKey)->unSerialize(node.get_child("MasterCardKey"), "");
	}

	std::string DESFireAccessInfo::getDefaultXmlNodeName() const
	{
		return "DESFireAccessInfo";
	}

	bool DESFireAccessInfo::operator==(const AccessInfo& ai) const
	{
		if (!AccessInfo::operator==(ai))
			return false;

		const DESFireAccessInfo* dfAi = dynamic_cast<const DESFireAccessInfo*>(&ai);

		return (readKey == dfAi->readKey
			&& readKeyno == dfAi->readKeyno
			&& writeKey == dfAi->writeKey
			&& writeKeyno == dfAi->writeKeyno
			&& masterApplicationKey == dfAi->masterApplicationKey
			&& masterCardKey == dfAi->masterCardKey
			);
	}
}

