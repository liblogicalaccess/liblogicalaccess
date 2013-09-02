/**
 * \file MifarePlusAccessInfo.cpp
 * \author Xavier SCHMERBER <xavier.schmerber@gmail.com>
 * \brief Access informations for mifarePlus card.
 */

#include "mifareplusaccessinfo.hpp"
#include "mifarepluschip.hpp"

namespace logicalaccess
{
	MifarePlusAccessInfo::MifarePlusAccessInfo(size_t sectorKeySize)
	{
		d_sectorKeySize = sectorKeySize;

		keyA = boost::shared_ptr<MifarePlusKey>(new MifarePlusKey(d_sectorKeySize));
		keyA->fromString(MIFARE_PLUS_DEFAULT_AESKEY);
		keyB = boost::shared_ptr<MifarePlusKey>(new MifarePlusKey(d_sectorKeySize));
		keyB->fromString(MIFARE_PLUS_DEFAULT_AESKEY);
		keyOriginality = boost::shared_ptr<MifarePlusKey>(new MifarePlusKey(MIFARE_PLUS_AES_KEY_SIZE));
		keyOriginality->fromString(MIFARE_PLUS_DEFAULT_AESKEY);
		keyMastercard = boost::shared_ptr<MifarePlusKey>(new MifarePlusKey(MIFARE_PLUS_AES_KEY_SIZE));
		keyMastercard->fromString(MIFARE_PLUS_DEFAULT_AESKEY);
		keyConfiguration = boost::shared_ptr<MifarePlusKey>(new MifarePlusKey(MIFARE_PLUS_AES_KEY_SIZE));
		keyConfiguration->fromString(MIFARE_PLUS_DEFAULT_AESKEY);
		keySwitchL2 = boost::shared_ptr<MifarePlusKey>(new MifarePlusKey(MIFARE_PLUS_AES_KEY_SIZE));
		keySwitchL2->fromString(MIFARE_PLUS_DEFAULT_AESKEY);
		keySwitchL3 = boost::shared_ptr<MifarePlusKey>(new MifarePlusKey(MIFARE_PLUS_AES_KEY_SIZE));
		keySwitchL3->fromString(MIFARE_PLUS_DEFAULT_AESKEY);
		keyAuthenticateSL1AES = boost::shared_ptr<MifarePlusKey>(new MifarePlusKey(MIFARE_PLUS_AES_KEY_SIZE));
		keyAuthenticateSL1AES->fromString(MIFARE_PLUS_DEFAULT_AESKEY);

		madKeyA = boost::shared_ptr<MifarePlusKey>(new MifarePlusKey(d_sectorKeySize));
		madKeyA->fromString(MIFARE_PLUS_DEFAULT_AESKEY);
		madKeyB = boost::shared_ptr<MifarePlusKey>(new MifarePlusKey(d_sectorKeySize));
		madKeyB->fromString(MIFARE_PLUS_DEFAULT_AESKEY);
		useMAD = false;
		sab.setTransportConfiguration();

		gpb = 0x00;
		madGPB = 0xC3; // DA: 1, MA: 1, RFU, ADV: 01
	}

	MifarePlusAccessInfo::~MifarePlusAccessInfo()
	{		
	}

	std::string MifarePlusAccessInfo::getCardType() const
	{
		return CHIP_MIFAREPLUS4K;
	}

	void MifarePlusAccessInfo::generateInfos()
	{		
		keyA->fromString(generateSimpleKey(0, keyA->getLength()));
		keyB->fromString(generateSimpleKey(1000, keyB->getLength()));
		keyOriginality->fromString(generateSimpleKey(2000, keyOriginality->getLength()));
		keyMastercard->fromString(generateSimpleKey(3000, keyMastercard->getLength()));
		keyConfiguration->fromString(generateSimpleKey(4000, keyConfiguration->getLength()));
		keySwitchL2->fromString(generateSimpleKey(5000, keySwitchL2->getLength()));
		keySwitchL3->fromString(generateSimpleKey(6000, keySwitchL3->getLength()));
		keyAuthenticateSL1AES->fromString(generateSimpleKey(7000, keyAuthenticateSL1AES->getLength()));

		sab.setAReadBWriteConfiguration();
	}

	void MifarePlusAccessInfo::generateInfos(const string& csn)
	{
		long lcsn = atol(csn.c_str());

		keyA->fromString(generateSimpleKey(lcsn, keyA->getLength()));
		keyB->fromString(generateSimpleKey(lcsn + 1000, keyB->getLength()));
		keyOriginality->fromString(generateSimpleKey(lcsn + 2000, keyOriginality->getLength()));
		keyMastercard->fromString(generateSimpleKey(lcsn + 3000, keyMastercard->getLength()));
		keyConfiguration->fromString(generateSimpleKey(lcsn + 4000, keyConfiguration->getLength()));
		keySwitchL2->fromString(generateSimpleKey(lcsn + 5000, keySwitchL2->getLength()));
		keySwitchL3->fromString(generateSimpleKey(lcsn + 6000, keySwitchL3->getLength()));
		keyAuthenticateSL1AES->fromString(generateSimpleKey(lcsn + 7000, keyAuthenticateSL1AES->getLength()));

		sab.setAReadBWriteConfiguration();
	}

	std::vector<unsigned char> MifarePlusAccessInfo::getLinearData() const
	{
		std::vector<unsigned char> data;
		
		data.push_back(static_cast<unsigned char>(keyA->isEmpty() ? 0 : 1));

		if (!keyA->isEmpty())
		{
			unsigned char* keydata = keyA->getData();
			data.insert(data.end(), keydata, keydata + keyA->getLength());
		}
		else
		{
			for (size_t i = 0; i < keyA->getLength(); ++i)
			{
				data.push_back(0x00);
			}
		}

		data.push_back(static_cast<unsigned char>(keyB->isEmpty() ? 0 : 1));

		if (!keyB->isEmpty())
		{
			unsigned char* keydata = keyB->getData();
			data.insert(data.end(), keydata, keydata + keyB->getLength());
		}
		else
		{
			for (size_t i = 0; i < keyB->getLength(); ++i)
			{
				data.push_back(0x00);
			}
		}

		data.push_back(static_cast<unsigned char>(keyOriginality->isEmpty() ? 0 : 1));

		if (!keyOriginality->isEmpty())
		{
			unsigned char* keydata = keyOriginality->getData();
			data.insert(data.end(), keydata, keydata + keyOriginality->getLength());
		}
		else
		{
			for (size_t i = 0; i < keyOriginality->getLength(); ++i)
			{
				data.push_back(0x00);
			}
		}

		data.push_back(static_cast<unsigned char>(keyMastercard->isEmpty() ? 0 : 1));

		if (!keyMastercard->isEmpty())
		{
			unsigned char* keydata = keyMastercard->getData();
			data.insert(data.end(), keydata, keydata + keyMastercard->getLength());
		}
		else
		{
			for (size_t i = 0; i < keyMastercard->getLength(); ++i)
			{
				data.push_back(0x00);
			}
		}

		data.push_back(static_cast<unsigned char>(keyConfiguration->isEmpty() ? 0 : 1));

		if (!keyConfiguration->isEmpty())
		{
			unsigned char* keydata = keyConfiguration->getData();
			data.insert(data.end(), keydata, keydata + keyConfiguration->getLength());
		}
		else
		{
			for (size_t i = 0; i < keyConfiguration->getLength(); ++i)
			{
				data.push_back(0x00);
			}
		}

		data.push_back(static_cast<unsigned char>(keySwitchL2->isEmpty() ? 0 : 1));

		if (!keySwitchL2->isEmpty())
		{
			unsigned char* keydata = keySwitchL2->getData();
			data.insert(data.end(), keydata, keydata + keySwitchL2->getLength());
		}
		else
		{
			for (size_t i = 0; i < keySwitchL2->getLength(); ++i)
			{
				data.push_back(0x00);
			}
		}

		data.push_back(static_cast<unsigned char>(keySwitchL3->isEmpty() ? 0 : 1));

		if (!keySwitchL3->isEmpty())
		{
			unsigned char* keydata = keySwitchL3->getData();
			data.insert(data.end(), keydata, keydata + keySwitchL3->getLength());
		}
		else
		{
			for (size_t i = 0; i < keySwitchL3->getLength(); ++i)
			{
				data.push_back(0x00);
			}
		}

		data.push_back(static_cast<unsigned char>(keyAuthenticateSL1AES->isEmpty() ? 0 : 1));

		if (!keyAuthenticateSL1AES->isEmpty())
		{
			unsigned char* keydata = keyAuthenticateSL1AES->getData();
			data.insert(data.end(), keydata, keydata + keyAuthenticateSL1AES->getLength());
		}
		else
		{
			for (size_t i = 0; i < keyAuthenticateSL1AES->getLength(); ++i)
			{
				data.push_back(0x00);
			}
		}

		unsigned char sabbuf[3];
		memset(sabbuf, 0x00, sizeof(sabbuf));
		sab.toArray(sabbuf, sizeof(sabbuf));
		data.insert(data.end(), sabbuf, sabbuf + sizeof(sabbuf));
		data.push_back(useMAD ? 0x01 : 0x00);

		data.push_back(static_cast<unsigned char>(madKeyA->isEmpty() ? 0 : 1));

		if (!madKeyA->isEmpty())
		{
			unsigned char* keydata = madKeyA->getData();
			data.insert(data.end(), keydata, keydata + madKeyA->getLength());
		}
		else
		{
			for (size_t i = 0; i < madKeyA->getLength(); ++i)
			{
				data.push_back(0x00);
			}
		}

		data.push_back(static_cast<unsigned char>(madKeyB->isEmpty() ? 0 : 1));

		if (!madKeyB->isEmpty())
		{
			unsigned char* keydata = madKeyB->getData();
			data.insert(data.end(), keydata, keydata + madKeyB->getLength());
		}
		else
		{
			for (size_t i = 0; i < madKeyB->getLength(); ++i)
			{
				data.push_back(0x00);
			}
		}
		
		return data;
	}

	void MifarePlusAccessInfo::setLinearData(const std::vector<unsigned char>& data, size_t offset)
	{
		bool hasKeyA = (data[offset++] == 1);

		if (hasKeyA)
		{
			keyA->setData(data, offset);
		}
		else
		{
			keyA->setData(std::vector<unsigned char>());
		}
		offset += keyA->getLength();

		bool hasKeyB = (data[offset++] == 1);

		if (hasKeyB)
		{
			keyB->setData(data, offset);
		}
		else
		{
			keyB->setData(std::vector<unsigned char>());
		}
		offset += keyB->getLength();

		bool haskeyOriginality = (data[offset++] == 1);

		if (haskeyOriginality)
		{
			keyOriginality->setData(data, offset);
		}
		else
		{
			keyOriginality->setData(std::vector<unsigned char>());
		}
		offset += keyOriginality->getLength();

		bool haskeyMastercard = (data[offset++] == 1);

		if (haskeyMastercard)
		{
			keyMastercard->setData(data, offset);
		}
		else
		{
			keyMastercard->setData(std::vector<unsigned char>());
		}
		offset += keyMastercard->getLength();

		bool haskeyConfiguration = (data[offset++] == 1);

		if (haskeyConfiguration)
		{
			keyConfiguration->setData(data, offset);
		}
		else
		{
			keyConfiguration->setData(std::vector<unsigned char>());
		}
		offset += keyConfiguration->getLength();

		bool haskeySwitchL2 = (data[offset++] == 1);

		if (haskeySwitchL2)
		{
			keySwitchL2->setData(data, offset);
		}
		else
		{
			keySwitchL2->setData(std::vector<unsigned char>());
		}
		offset += keySwitchL2->getLength();

		bool haskeySwitchL3 = (data[offset] == 1);

		if (haskeySwitchL3)
		{
			keySwitchL3->setData(data, offset);
		}
		else
		{
			keySwitchL3->setData(std::vector<unsigned char>());
		}
		offset += keySwitchL3->getLength();

		bool haskeyAuthenticateSL1AES = (data[offset] == 1);

		if (haskeyAuthenticateSL1AES)
		{
			keyAuthenticateSL1AES->setData(data, offset);
		}
		else
		{
			keyAuthenticateSL1AES->setData(std::vector<unsigned char>());
		}
		offset += keyAuthenticateSL1AES->getLength();

		sab.fromArray(&data[offset], 3);
		offset += 3;
		useMAD = (data[offset++] == 0x01);

		bool hasMadKeyA = (data[offset++] == 1);

		if (hasMadKeyA)
		{
			madKeyA->setData(data, offset);
		}
		else
		{
			madKeyA->setData(std::vector<unsigned char>());
		}
		offset += madKeyA->getLength();

		bool hasMadKeyB = (data[offset] == 1);

		if (hasMadKeyB)
		{
			madKeyB->setData(data, offset);
		}
		else
		{
			madKeyB->setData(std::vector<unsigned char>());
		}
		offset += madKeyB->getLength();
	}

	size_t MifarePlusAccessInfo::getDataSize()
	{
		return (keyA->getLength() + 1 +
				keyB->getLength() + 1 +
				keyOriginality->getLength() + 1 +
				keyMastercard->getLength() + 1 +
				keyConfiguration->getLength() + 1 +
				keySwitchL2->getLength() + 1 +
				keySwitchL3->getLength() + 1 +
				keyAuthenticateSL1AES->getLength() + 1 +
				madKeyA->getLength() + 1 +
				madKeyB->getLength() + 1 +
				sizeof(sab) + sizeof(useMAD));
	}

	void MifarePlusAccessInfo::serialize(boost::property_tree::ptree& parentNode)
	{
		unsigned char buf[3];
		boost::property_tree::ptree node;
		sab.toArray(buf, sizeof(buf));
		std::ostringstream oss;		
		oss << std::setfill('0');
		for (size_t i = 0; i < sizeof(buf); ++i)
		{
			oss << std::setw(2) << std::hex << static_cast<unsigned int>(buf[i]);
			if (i < sizeof(buf) - 1)
			{
				oss << " ";
			}
		}

		boost::property_tree::ptree ka;
		keyA->serialize(ka);
		node.add_child("KeyA", ka);
		boost::property_tree::ptree kb;
		keyB->serialize(kb);
		node.add_child("KeyB", kb);
		boost::property_tree::ptree ko;
		keyOriginality->serialize(ko);
		node.add_child("KeyOriginality", ko);
		boost::property_tree::ptree km;
		keyMastercard->serialize(km);
		node.add_child("KeyMastercard", km);
		boost::property_tree::ptree kc;
		keyConfiguration->serialize(kc);
		node.add_child("KeyConfiguration", kc);
		boost::property_tree::ptree ks2;
		keySwitchL2->serialize(ks2);
		node.add_child("KeySwitchL2", ks2);
		boost::property_tree::ptree ks3;
		keySwitchL3->serialize(ks3);
		node.add_child("KeySwitchL3", ks3);
		boost::property_tree::ptree kaa;
		keyAuthenticateSL1AES->serialize(kaa);
		node.add_child("KeyAuthenticateSL1AES", kaa);

		node.put("SectorAccessBits", oss.str());

		boost::property_tree::ptree madnode;
		madnode.put("Use", useMAD);
		boost::property_tree::ptree mada;
		madKeyA->serialize(mada);
		madnode.add_child("MADKeyA", mada);
		boost::property_tree::ptree madb;
		madKeyB->serialize(madb);
		madnode.add_child("MADKeyB", madb);
		node.add_child("MAD", madnode);

		parentNode.add_child(getDefaultXmlNodeName(), node);
	}

	void MifarePlusAccessInfo::unSerialize(boost::property_tree::ptree& node)
	{
		boost::dynamic_pointer_cast<XmlSerializable>(keyA)->unSerialize(node.get_child("KeyA"), "");
		boost::dynamic_pointer_cast<XmlSerializable>(keyB)->unSerialize(node.get_child("KeyB"), "");
		boost::dynamic_pointer_cast<XmlSerializable>(keyOriginality)->unSerialize(node.get_child("KeyOriginality"), "");
		boost::dynamic_pointer_cast<XmlSerializable>(keyMastercard)->unSerialize(node.get_child("KeyMastercard"), "");
		boost::dynamic_pointer_cast<XmlSerializable>(keyConfiguration)->unSerialize(node.get_child("KeyConfiguration"), "");
		boost::dynamic_pointer_cast<XmlSerializable>(keySwitchL2)->unSerialize(node.get_child("KeySwitchL2"), "");
		boost::dynamic_pointer_cast<XmlSerializable>(keySwitchL3)->unSerialize(node.get_child("KeySwitchL3"), "");
		boost::dynamic_pointer_cast<XmlSerializable>(keyAuthenticateSL1AES)->unSerialize(node.get_child("KeyAuthenticateSL1AES"), "");

		string sabstr = node.get_child("SectorAccessBits").get_value<std::string>();
		unsigned char buf[3];
		memset(buf, 0x00, sizeof(buf));
		std::istringstream iss(sabstr);
		for (size_t i = 0; i < sizeof(buf); ++i)
		{
			unsigned int tmp;
			if (!iss.good())
			{
				return;
			}
			iss >> std::hex >> tmp;

			buf[i] = static_cast<unsigned char>(tmp);
		}
		sab.fromArray(buf, sizeof(buf));

		boost::property_tree::ptree modnode = node.get_child("MAD");
		if (!modnode.empty())
		{
			useMAD = modnode.get_child("Use").get_value<bool>();
			boost::dynamic_pointer_cast<XmlSerializable>(madKeyA)->unSerialize(modnode.get_child("MADKeyA"), "");
			boost::dynamic_pointer_cast<XmlSerializable>(madKeyB)->unSerialize(modnode.get_child("MADKeyB"), "");
		}
	}

	std::string MifarePlusAccessInfo::getDefaultXmlNodeName() const
	{
		return "MifarePlusAccessInfo";
	}

	bool MifarePlusAccessInfo::operator==(const AccessInfo& ai) const
	{
		if (!AccessInfo::operator==(ai))
			return false;

		const MifarePlusAccessInfo* mAi = dynamic_cast<const MifarePlusAccessInfo*>(&ai);

		return (keyA == mAi->keyA
			&& keyB == mAi->keyB
			&& keyOriginality == mAi->keyOriginality
			&& keyMastercard == mAi->keyMastercard
			&& keyConfiguration == mAi->keyConfiguration
			&& keySwitchL2 == mAi->keySwitchL2
			&& keySwitchL3 == mAi->keySwitchL3
			&& keyAuthenticateSL1AES == mAi->keyAuthenticateSL1AES
			&& madKeyA == mAi->madKeyA
			&& madKeyB == mAi->madKeyB
			&& useMAD == mAi->useMAD
			);
	}

	size_t MifarePlusAccessInfo::SectorAccessBits::toArray(void* buf, size_t buflen) const
	{
		if (buflen < 3)
		{
			throw EXCEPTION(std::invalid_argument, "The buffer is too short.");
		}

		memset(buf, 0x00, 3);

		unsigned char* data = reinterpret_cast<unsigned char*>(buf);

		data[1] = (d_sector_trailer_access_bits.c1 << 7) | (d_data_blocks_access_bits[2].c1 << 6) | (d_data_blocks_access_bits[1].c1 << 5) | (d_data_blocks_access_bits[0].c1 << 4);
		data[2] = (d_sector_trailer_access_bits.c3 << 7) | (d_data_blocks_access_bits[2].c3 << 6) | (d_data_blocks_access_bits[1].c3 << 5) | (d_data_blocks_access_bits[0].c3 << 4);
		data[2] |= (d_sector_trailer_access_bits.c2 << 3) | (d_data_blocks_access_bits[2].c2 << 2) | (d_data_blocks_access_bits[1].c2 << 1) | (d_data_blocks_access_bits[0].c2 << 0);
		data[0] = ((~data[2]) << 4) & 0xF0;
		data[0] |= ((~data[1]) >> 4) & 0x0F;
		data[1] |= ((~data[2]) >> 4) & 0x0F;

		return 3;
	}

	bool MifarePlusAccessInfo::SectorAccessBits::fromArray(const void* buf, size_t buflen)
	{
		if (buflen < 3)
		{
			throw EXCEPTION(std::invalid_argument, "The buffer is too short.");
		}

		SectorAccessBits sab;

		const unsigned char* data = reinterpret_cast<const unsigned char*>(buf);

		sab.d_sector_trailer_access_bits.c1 = ((data[1] & 0x80) != 0);
		sab.d_sector_trailer_access_bits.c2 = ((data[2] & 0x08) != 0);
		sab.d_sector_trailer_access_bits.c3 = ((data[2] & 0x80) != 0);
		sab.d_data_blocks_access_bits[2].c1 = ((data[1] & 0x40) != 0);
		sab.d_data_blocks_access_bits[2].c2 = ((data[2] & 0x04) != 0);
		sab.d_data_blocks_access_bits[2].c3 = ((data[2] & 0x40) != 0);
		sab.d_data_blocks_access_bits[1].c1 = ((data[1] & 0x20) != 0);
		sab.d_data_blocks_access_bits[1].c2 = ((data[2] & 0x02) != 0);
		sab.d_data_blocks_access_bits[1].c3 = ((data[2] & 0x20) != 0);
		sab.d_data_blocks_access_bits[0].c1 = ((data[1] & 0x10) != 0);
		sab.d_data_blocks_access_bits[0].c2 = ((data[2] & 0x01) != 0);
		sab.d_data_blocks_access_bits[0].c3 = ((data[2] & 0x10) != 0);

		unsigned char test[3];

		if (sab.toArray(test, 3) > 0)
		{
			if (memcmp(test, buf, 3) == 0)
			{
				operator=(sab);

				return true;
			}
		}

		return false;
	}

	void MifarePlusAccessInfo::SectorAccessBits::setTransportConfiguration()
	{
		d_sector_trailer_access_bits.c1 = false;
		d_sector_trailer_access_bits.c2 = false;
		d_sector_trailer_access_bits.c3 = true;

		for (unsigned int i = 0; i < 3; ++i)
		{
			d_data_blocks_access_bits[i].c1 = false;
			d_data_blocks_access_bits[i].c2 = false;
			d_data_blocks_access_bits[i].c3 = false;
		}
	}

	void MifarePlusAccessInfo::SectorAccessBits::setAReadBWriteConfiguration()
	{
		d_sector_trailer_access_bits.c1 = false;
		d_sector_trailer_access_bits.c2 = true;
		d_sector_trailer_access_bits.c3 = true;

		for (unsigned int i = 0; i < 3; ++i)
		{
			d_data_blocks_access_bits[i].c1 = true;
			d_data_blocks_access_bits[i].c2 = false;
			d_data_blocks_access_bits[i].c3 = false;
		}
	}
}