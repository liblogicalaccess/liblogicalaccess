/**
 * \file MifareUltralightCAccessInfo.cpp
 * \author Maxime CHAMLEY <maxime.chamley@islog.eu>
 * \brief Access informations for Mifare Ultralight C card.
 */

#include "MifareUltralightCAccessInfo.h"

namespace LOGICALACCESS
{
	MifareUltralightCAccessInfo::MifareUltralightCAccessInfo()
		: MifareUltralightAccessInfo()
	{
		key.reset(new TripleDESKey());
	}

	MifareUltralightCAccessInfo::~MifareUltralightCAccessInfo()
	{		
	}

	void MifareUltralightCAccessInfo::generateInfos()
	{		
		MifareUltralightAccessInfo::generateInfos();

		key->fromString(generateSimpleKey(0, TRIPLEDES_KEY_SIZE));
	}

	void MifareUltralightCAccessInfo::generateInfos(const string& csn)
	{
		MifareUltralightAccessInfo::generateInfos(csn);

		long lcsn = atol(csn.c_str());
		key->fromString(generateSimpleKey(lcsn, TRIPLEDES_KEY_SIZE));
	}

	std::vector<unsigned char> MifareUltralightCAccessInfo::getLinearData() const
	{
		std::vector<unsigned char> data = MifareUltralightAccessInfo::getLinearData();

		data.push_back(key->isEmpty() ? 0 : 1);

		if (!key->isEmpty())
		{
			unsigned char* keydata = key->getData();
			data.insert(data.end(), keydata, keydata + key->getLength());
		}
		else
		{
			for (size_t i = 0; i < key->getLength(); ++i)
			{
				data.push_back(0x00);
			}
		}

		return data;
	}

	void MifareUltralightCAccessInfo::setLinearData(const std::vector<unsigned char>& data, size_t offset)
	{
		MifareUltralightAccessInfo::setLinearData(data, offset);
		offset += MifareUltralightAccessInfo::getDataSize();

		bool hasKey = (data[offset++] == 1);

		if (hasKey)
		{
			key->setData(data, offset);
		}
		else
		{
			key->setData(std::vector<unsigned char>());
		}
		offset += key->getLength();
	}

	size_t MifareUltralightCAccessInfo::getDataSize()
	{
		return (MifareUltralightAccessInfo::getDataSize() + 1 + TRIPLEDES_KEY_SIZE);
	}

	void MifareUltralightCAccessInfo::serialize(boost::property_tree::ptree& parentNode)
	{
		boost::property_tree::ptree node;
		MifareUltralightAccessInfo::serialize(node);

		boost::property_tree::ptree keynode;
		key->serialize(keynode);
		node.add_child("Key", keynode);

		parentNode.add_child(getDefaultXmlNodeName(), node);
	}

	void MifareUltralightCAccessInfo::unSerialize(boost::property_tree::ptree& node)
	{
		MifareUltralightAccessInfo::unSerialize(node.get_child(MifareUltralightAccessInfo::getDefaultXmlNodeName()));

		key->unSerialize(node.get_child("Key"));
	}

	std::string MifareUltralightCAccessInfo::getDefaultXmlNodeName() const
	{
		return "MifareUltralightCAccessInfo";
	}

	bool MifareUltralightCAccessInfo::operator==(const AccessInfo& ai) const
	{
		if (!MifareUltralightAccessInfo::operator==(ai))
			return false;

		const MifareUltralightCAccessInfo* mAi = dynamic_cast<const MifareUltralightCAccessInfo*>(&ai);

		return (key == mAi->key
			);
	}
}

