/**
 * \file desfirekey.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief DESFire Key.
 */

#include "samav2keyentry.hpp"

namespace logicalaccess
{
	SAMAV2KeyEntry::SAMAV2KeyEntry() : d_updatemask(0)
	{
		d_keyType = SAMAV2_KEY_DES;
		d_key.reset(new unsigned char[getLength()]);
		memset(&*d_key, 0, getLength());
		d_diversify = false;
		d_updatemask = 0;
		d_keyentryinformation.reset(new KeyEntryInformation());
		memset(&*d_keyentryinformation, 0, sizeof(KeyEntryInformation));
		memset(d_keyentryinformation->set, 0 , 2);
	}

	SAMAV2KeyEntry::SAMAV2KeyEntry(const std::string& str, const std::string& str1, const std::string& str2) : d_updatemask(0)
	{
		d_keyType = SAMAV2_KEY_DES;
		d_key.reset(new unsigned char[getLength()]);
		memset(&*d_key, 0, getLength());
		d_diversify = false;
		d_updatemask = 0;
		d_keyentryinformation.reset(new KeyEntryInformation());
		memset(&*d_keyentryinformation, 0, sizeof(KeyEntryInformation));
		memset(d_keyentryinformation->set, 0 , 2);
	}

	SAMAV2KeyEntry::SAMAV2KeyEntry(const void** buf, size_t buflen, char numberkey) : d_updatemask(0)
	{
		d_keyType = SAMAV2_KEY_DES;
		d_key.reset(new unsigned char[getLength()]);
		memset(&*d_key, 0, getLength());
		d_diversify = false;
		d_updatemask = 0;
		d_keyentryinformation.reset(new KeyEntryInformation());
		memset(&*d_keyentryinformation, 0, sizeof(KeyEntryInformation));
		memset(d_keyentryinformation->set, 0 , 2);
		if (buf != NULL)
		{
			if (buflen * numberkey  >= getLength())
			{
				if (numberkey >= 1)
				{
					memset(&*d_key, 0, getLength());
					memcpy(&*d_key, buf[0], getSingleLength());
				}
				if (numberkey >= 2)
				{
					memset(&*d_key, 0, getLength());
					memcpy(&*d_key + getSingleLength(), buf[1], getSingleLength());
				}
				if (numberkey >= 3)
				{
					memset(&*d_key, 0, getLength());
					memcpy(&*d_key + (getSingleLength() * 2), buf[2], getSingleLength());
				}
			}
		}
	}

	size_t SAMAV2KeyEntry::getSingleLength() const
	{
		size_t length = 0;

		switch (d_keyType)
		{
		case SAMAV2_KEY_DES:
			length = SAMAV2_DES_KEY_SIZE;
			break;

		case SAMAV2_KEY_3K3DES:
			length = SAMAV2_MAXKEY_SIZE;
			break;

		case SAMAV2_KEY_AES:
			length = SAMAV2_AES_KEY_SIZE;
			break;
		}

		return length;
	}

	size_t SAMAV2KeyEntry::getLength() const
	{
		size_t length = 0;

		switch (d_keyType)
		{
		case SAMAV2_KEY_DES:
			length = SAMAV2_DES_KEY_SIZE * 3;
			break;

		case SAMAV2_KEY_3K3DES:
			length = SAMAV2_MAXKEY_SIZE * 2;
			break;

		case SAMAV2_KEY_AES:
			length = SAMAV2_AES_KEY_SIZE * 3;
			break;
		}

		return length;
	}

	unsigned char** SAMAV2KeyEntry::getKey()
	{
		unsigned char **keys;
		size_t keysize = getSingleLength();
		unsigned char keynb = 3;

		if (d_keyType == SAMAV2_MAXKEY_SIZE)
		{
			keynb = 2;
			keys = new unsigned char*[2];
		}
		else
			keys = new unsigned char*[3];

		for (unsigned char x = 0; x < keynb; ++x)
		{
			keys[x] = new unsigned char[keysize];
			memcpy(keys[x], &*d_key + (x * keysize), keysize); 
		}
		return keys;
	}

	void		SAMAV2KeyEntry::setKeyTypeFromSET()
	{
		char keytype = 0x1c & d_keyentryinformation->set[1];
		size_t oldsize = getLength();

		switch (keytype)
		{
		case SAMAV2_KEY_DES:
			d_keyType = SAMAV2_KEY_DES;
			break;

		case SAMAV2_KEY_3K3DES:
			d_keyType = SAMAV2_KEY_3K3DES;
			break;

		case SAMAV2_KEY_AES:
			d_keyType = SAMAV2_KEY_AES;
			break;
		}

		unsigned char *tmp = new unsigned char[getLength()];
		if (getLength() < oldsize)
			oldsize = getLength();
		memcpy(tmp, &*d_key, oldsize);
		d_key.reset(tmp);
	}

	void SAMAV2KeyEntry::serialize(boost::property_tree::ptree& parentNode)
	{
		boost::property_tree::ptree node;

		node.put("KeyType", d_keyType);
		node.put("Diversify", d_diversify);

		parentNode.add_child(getDefaultXmlNodeName(), node);
	}

	void SAMAV2KeyEntry::unSerialize(boost::property_tree::ptree& node)
	{
		d_keyType = static_cast<SAMAV2KeyType>(node.get_child("KeyType").get_value<unsigned int>());
		d_diversify = node.get_child("Diversify").get_value<bool>();
	}

	std::string SAMAV2KeyEntry::getDefaultXmlNodeName() const
	{
		return "SAMAV2KeyEntry";
	}

	bool SAMAV2KeyEntry::operator==(const SAMAV2KeyEntry& key) const
	{
		if (d_keyType != key.d_keyType)
		{
			return false;
		}
		if (memcmp(&*d_key, &*(key.d_key), getLength()) == 0)
		{
			return false;
		}
		return true;
	}

	std::string SAMAV2KeyEntry::SAMAV2KeyEntryTypeStr(SAMAV2KeyType t)
	{
		switch (t) {
			case SAMAV2_KEY_DES: return "SAMAV2_KEY_DES";
			case SAMAV2_KEY_3K3DES: return "SAMAV2_KEY_3K3DES";
			case SAMAV2_KEY_AES: return "SAMAV2_KEY_AES";
			default: return "Unknown";
		}
	}

	boost::shared_ptr<KeyEntryUpdateSettings> SAMAV2KeyEntry::getUpdateSettings()
	{
		KeyEntryUpdateSettings *settings = new KeyEntryUpdateSettings;

		bool *x = (bool*)settings;
		for (unsigned char i = 0; i < sizeof(*settings); ++i)
		{
			if ((d_updatemask & 0x80) == 0x80)
				x[i] = 1;
			else
				x[i] = 0;
			if (i + 1 < sizeof(*settings))
				d_updatemask = d_updatemask << 1;
		}
		return boost::shared_ptr<KeyEntryUpdateSettings>(settings);
	}

	void SAMAV2KeyEntry::setUpdateSettings(boost::shared_ptr<KeyEntryUpdateSettings> t)
	{
		bool *x = (bool*)&*t;
		d_updatemask = 0;
		for (unsigned char i = 0; i < sizeof(KeyEntryUpdateSettings); ++i)
		{
			d_updatemask += x[i];
			if (i + 1 < sizeof(KeyEntryUpdateSettings))
				d_updatemask = d_updatemask << 1;
		}
	}

	boost::shared_ptr<SET> SAMAV2KeyEntry::getSETStruct()
	{
		SET *set = new SET;
		bool *x = (bool*)set;
		unsigned char j = 0;
		for (unsigned char i = 0; i < sizeof(*set); ++i)
		{
			if ((d_keyentryinformation->set[j] & 0x80) == 0x80)
				x[i] = 1;
			else
				x[i] = 0;
			if (i + 1 < sizeof(*set))
				d_keyentryinformation->set[j] = d_keyentryinformation->set[j] << 1;
			if (i == 8)
				j++;
		}
		return boost::shared_ptr<SET>(set);
	}

	void SAMAV2KeyEntry::setSET(SET t)
	{
		bool *x = (bool*)&t;
		memset(d_keyentryinformation->set, 0, 2);
		unsigned char j = 0;
		for (unsigned char i = 0; i < sizeof(t); ++i)
		{
			d_keyentryinformation->set[j] += (char)x[i];
			if (i + 1 < sizeof(t))
				d_keyentryinformation->set[j] = d_keyentryinformation->set[j] << 1;
			if (i == 8)
				j++;
		}
	}
}