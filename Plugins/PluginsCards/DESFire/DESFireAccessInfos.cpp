/**
 * \file desfireaccessinfo.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Access informations for DESFire card.
 */

#include "desfireaccessinfo.hpp"
#include "desfireprofile.hpp"

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

	void DESFireAccessInfo::generateInfos()
	{
		INFO_SIMPLE_("Generate access information");
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

	std::vector<unsigned char> DESFireAccessInfo::getLinearData() const
	{
		INFO_SIMPLE_("Get linear data...");
		std::vector<unsigned char> data(128);
		
		data.push_back(static_cast<unsigned char>(readKey->getKeyType()));
		data.push_back(readKey->isEmpty() ? 0 : 1);

		if (!readKey->isEmpty())
		{
			unsigned char* keydata = readKey->getData();
			data.insert(data.end(), keydata, keydata + readKey->getLength());
		}
		else
		{
			for (size_t i = 0; i < readKey->getLength(); ++i)
			{
				data.push_back(0x00);
			}
		}
		data.push_back(readKey->getKeyVersion());
		data.push_back(readKey->getDiversify() ? 1 : 0);
		data.push_back(readKeyno);

		data.push_back(static_cast<unsigned char>(writeKey->getKeyType()));
		data.push_back(writeKey->isEmpty() ? 0 : 1);

		if (!writeKey->isEmpty())
		{
			unsigned char* keydata = writeKey->getData();
			data.insert(data.end(), keydata, keydata + writeKey->getLength());
		}
		else
		{
			for (size_t i = 0; i < writeKey->getLength(); ++i)
			{
				data.push_back(0x00);
			}
		}
		data.push_back(writeKey->getKeyVersion());
		data.push_back(writeKey->getDiversify() ? 1 : 0);
		data.push_back(writeKeyno);
		
		data.push_back(static_cast<unsigned char>(masterApplicationKey->getKeyType()));
		data.push_back(masterApplicationKey->isEmpty() ? 0 : 1);

		if (!masterApplicationKey->isEmpty())
		{
			unsigned char* keydata = masterApplicationKey->getData();
			data.insert(data.end(), keydata, keydata + masterApplicationKey->getLength());
		}
		else
		{
			for (size_t i = 0; i < masterApplicationKey->getLength(); ++i)
			{
				data.push_back(0x00);
			}
		}
		data.push_back(masterApplicationKey->getKeyVersion());
		data.push_back(masterApplicationKey->getDiversify() ? 1 : 0);

		data.push_back(static_cast<unsigned char>(masterCardKey->getKeyType()));
		data.push_back(masterCardKey->isEmpty() ? 0 : 1);

		if (!masterCardKey->isEmpty())
		{
			unsigned char* keydata = masterCardKey->getData();
			data.insert(data.end(), keydata, keydata + masterCardKey->getLength());
		}
		else
		{
			for (size_t i = 0; i < masterCardKey->getLength(); ++i)
			{
				data.push_back(0x00);
			}
		}
		data.push_back(masterCardKey->getKeyVersion());
		data.push_back(masterCardKey->getDiversify() ? 1 : 0);

		return data;
	}

	void DESFireAccessInfo::setLinearData(const std::vector<unsigned char>& data, size_t offset)
	{
		INFO_SIMPLE_("Set linear data...");

		readKey->setKeyType(static_cast<DESFireKeyType>(data.at(offset++)));
		bool hasReadKey = (data.at(offset++) == 1);

		if (hasReadKey)
		{
			readKey->setData(data, offset);
		}
		else
		{
			readKey->setData(std::vector<unsigned char>());
		}
		offset += readKey->getLength();
		readKey->setKeyVersion(data.at(offset++));
		readKey->setDiversify(data.at(offset++) == 1);
		readKeyno = data.at(offset++);

		writeKey->setKeyType(static_cast<DESFireKeyType>(data.at(offset++)));
		bool hasWriteKey = (data.at(offset++) == 1);

		if (hasWriteKey)
		{
			writeKey->setData(data, offset);
		}
		else
		{
			writeKey->setData(std::vector<unsigned char>());
		}
		offset += writeKey->getLength();
		writeKey->setKeyVersion(data.at(offset++));
		writeKey->setDiversify(data.at(offset++) == 1);
		writeKeyno = data.at(offset++);

		masterApplicationKey->setKeyType(static_cast<DESFireKeyType>(data.at(offset++)));
		bool hasMasterApplicationKey = (data.at(offset++) == 1);

		if (hasMasterApplicationKey)
		{
			masterApplicationKey->setData(data, offset);
		}
		else
		{
			masterApplicationKey->setData(std::vector<unsigned char>());
		}
		offset += masterApplicationKey->getLength();
		masterApplicationKey->setKeyVersion(data.at(offset++));
		masterApplicationKey->setDiversify(data.at(offset++) == 1);

		masterCardKey->setKeyType(static_cast<DESFireKeyType>(data.at(offset++)));
		bool hasMasterCardKey = (data.at(offset++) == 1);

		if (hasMasterCardKey)
		{
			masterCardKey->setData(data, offset);
		}
		else
		{
			masterCardKey->setData(std::vector<unsigned char>());
		}
		offset += masterCardKey->getLength();
		masterCardKey->setKeyVersion(data.at(offset++));
		masterCardKey->setDiversify(data.at(offset++) == 1);
	}

	size_t DESFireAccessInfo::getDataSize()
	{
		return readKey->getLength() + writeKey->getLength() + masterApplicationKey->getLength() + masterCardKey->getLength() + 2 + (4 * (1 + 1 + 1 + 1));
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

