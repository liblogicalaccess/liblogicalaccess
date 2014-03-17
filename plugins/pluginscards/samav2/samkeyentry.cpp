/**
 * \file desfirekey.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief DESFire Key.
 */

#include "samkeyentry.hpp"

namespace logicalaccess
{
	template <typename T, typename S> 
	SAMKeyEntry<T, S>::SAMKeyEntry(const SAMType samType) : d_updatemask(0), d_samType(samType)
	{
		d_keyType = SAM_KEY_DES;
		d_key = new unsigned char[getLength()];
		memset(&*d_key, 0, getLength());
		d_diversify = false;
		d_updatemask = 0;
		memset(&d_keyentryinformation, 0x00, sizeof(KeyEntryAV1Information));
	}

	template <typename T, typename S> 
	SAMKeyEntry<T, S>::SAMKeyEntry(const SAMType samType, const std::string& str, const std::string& str1, const std::string& str2) : d_updatemask(0), d_samType(samType)
	{
		d_keyType = SAM_KEY_DES;
		d_key = new unsigned char[getLength()];
		memset(&*d_key, 0, getLength());
		d_diversify = false;
		d_updatemask = 0;
		memset(&d_keyentryinformation, 0x00, sizeof(KeyEntryAV1Information));
		if (str != "")
			memcpy(d_key, str.c_str(), getSingleLength());
		if (str1 != "")
			memcpy(d_key, str1.c_str(), getSingleLength());
		if (str2 != "")
			memcpy(d_key, str2.c_str(), getSingleLength());

	}

	template <typename T, typename S> 
	SAMKeyEntry<T, S>::SAMKeyEntry(const SAMType samType, const void** buf, size_t buflen, char numberkey) : d_updatemask(0), d_samType(samType)
	{
		d_keyType = SAM_KEY_DES;
		d_key = new unsigned char[getLength()];
		memset(&*d_key, 0, getLength());
		d_diversify = false;
		d_updatemask = 0;
		memset(&d_keyentryinformation, 0x00, sizeof(KeyEntryAV1Information));
		if (buf != NULL)
		{
			if (buflen * numberkey  >= getLength())
			{
				if (numberkey >= 1)
				{
					memcpy(d_key, buf[0], getSingleLength());
				}
				if (numberkey >= 2)
				{
					memcpy(d_key + getSingleLength(), buf[1], getSingleLength());
				}
				if (numberkey >= 3)
				{
					memcpy(d_key + (getSingleLength() * 2), buf[2], getSingleLength());
				}
			}
		}
	}

	template <typename T, typename S> 
	SAMKeyEntry<T, S>::~SAMKeyEntry()
	{
		delete[] d_key;
	}

	template <typename T, typename S> 
	size_t SAMKeyEntry<T, S>::getSingleLength() const
	{
		size_t length = 0;

		switch (d_keyType)
		{
		case SAM_KEY_DES:
			length = SAM_DES_KEY_SIZE;
			break;

		case SAM_KEY_3K3DES:
			length = SAM_MAXKEY_SIZE;
			break;

		case SAM_KEY_AES:
			length = SAM_AES_KEY_SIZE;
			break;
		}

		return length;
	}

	template <typename T, typename S> 
	size_t SAMKeyEntry<T, S>::getLength() const
	{
		size_t length = 0;

		switch (d_keyType)
		{
		case SAM_KEY_DES:
			length = SAM_DES_KEY_SIZE * 3;
			break;

		case SAM_KEY_3K3DES:
			length = SAM_MAXKEY_SIZE * 2;
			break;

		case SAM_KEY_AES:
			length = SAM_AES_KEY_SIZE * 3;
			break;
		}

		return length;
	}

	template <typename T, typename S> 
	std::vector<std::vector<unsigned char> > SAMKeyEntry<T, S>::getKeysData()
	{
		std::vector<std::vector<unsigned char> > ret;
		size_t keysize = getSingleLength();
		unsigned char keynb = 3;

		if (d_keyType == SAM_MAXKEY_SIZE)
			keynb = 2;

		for (unsigned char x = 0; x < keynb; ++x)
			ret.push_back(std::vector<unsigned char>(d_key + (x * keysize), d_key + (x * keysize) + keysize));
		return ret;
	}

	template <typename T, typename S> 
	void SAMKeyEntry<T, S>::setKeysData(std::vector<std::vector<unsigned char> > keys, SAMKeyType type)
	{
		if (keys.size() == 0)
			return;
		unsigned char keynb = 3;
		d_keyType = type;

		if (d_keyType == SAM_KEY_3K3DES)
			keynb = 2;

		size_t keysize = getSingleLength();

		delete[] d_key;
		d_key = new unsigned char[getLength()];
		for (unsigned char x = 0; x < keynb; ++x)
		{
			if (keys[x].size() != keysize)
				THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "setKey failed because key don't have all the same size.");
			memcpy(d_key + (x * keysize), &keys[x][0], keysize);
		}
		setSETKeyTypeFromKeyType();
	}

	template <typename T, typename S> 
	void SAMKeyEntry<T, S>::setSETKeyTypeFromKeyType()
	{
		d_keyentryinformation.set[0] = d_keyentryinformation.set[0] - (0x38 & d_keyentryinformation.set[0]);
		switch (d_keyType)
		{
		case SAM_KEY_DES:
			break;

		case SAM_KEY_3K3DES:
			d_keyentryinformation.set[0] |= 0x18;
			break;

		case SAM_KEY_AES:
			d_keyentryinformation.set[0] |= 0x20;
			break;
		}
	}

	template <typename T, typename S> 
	void SAMKeyEntry<T, S>::setKeyTypeFromSET()
	{
		char keytype = 0x38 & d_keyentryinformation.set[0];
		size_t oldsize = getLength();

		switch (keytype)
		{
		case SAM_KEY_DES:
			d_keyType = SAM_KEY_DES;
			break;

		case SAM_KEY_3K3DES:
			d_keyType = SAM_KEY_3K3DES;
			break;

		case SAM_KEY_AES:
			d_keyType = SAM_KEY_AES;
			break;
		}

		unsigned char *tmp = new unsigned char[getLength()];
		if (getLength() < oldsize)
			oldsize = getLength();
		memcpy(tmp, &*d_key, oldsize);
		delete d_key;
		d_key = tmp;
	}

	template <typename T, typename S> 
	void SAMKeyEntry<T, S>::serialize(boost::property_tree::ptree& parentNode)
	{
		boost::property_tree::ptree node;

		node.put("KeyType", d_keyType);
		node.put("Diversify", d_diversify);

		parentNode.add_child(getDefaultXmlNodeName(), node);
	}

	template <typename T, typename S> 
	void SAMKeyEntry<T, S>::unSerialize(boost::property_tree::ptree& node)
	{
		d_keyType = static_cast<SAMKeyType>(node.get_child("KeyType").get_value<unsigned int>());
		d_diversify = node.get_child("Diversify").get_value<bool>();
	}

	template <typename T, typename S> 
	std::string SAMKeyEntry<T, S>::getDefaultXmlNodeName() const
	{
		return "SAMKeyEntry";
	}

	template <typename T, typename S> 
	bool SAMKeyEntry<T, S>::operator==(const SAMKeyEntry& key) const
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

	template <typename T, typename S> 
	std::string SAMKeyEntry<T, S>::SAMKeyEntryTypeStr(SAMKeyType t)
	{
		switch (t) {
			case SAM_KEY_DES: return "SAM_KEY_DES";
			case SAM_KEY_3K3DES: return "SAM_KEY_3K3DES";
			case SAM_KEY_AES: return "SAM_KEY_AES";
			default: return "Unknown";
		}
	}

	template <typename T, typename S> 
	KeyEntryUpdateSettings SAMKeyEntry<T, S>::getUpdateSettings()
	{
		KeyEntryUpdateSettings settings;

		bool *x = (bool*)&settings;
		for (unsigned char i = 0; i < sizeof(settings); ++i)
		{
			if ((d_updatemask & 0x80) == 0x80)
				x[i] = 1;
			else
				x[i] = 0;
			if (i + (unsigned int)1 < sizeof(settings))
				d_updatemask = d_updatemask << 1;
		}

		return settings;
	}

	template <typename T, typename S> 
	void SAMKeyEntry<T, S>::setUpdateSettings(const KeyEntryUpdateSettings& t)
	{
		bool *x = (bool*)&t;
		d_updatemask = 0;
		for (unsigned char i = 0; i < sizeof(KeyEntryUpdateSettings); ++i)
		{
			d_updatemask += x[i];
			if (i + (unsigned int)1 < sizeof(KeyEntryUpdateSettings))
				d_updatemask = d_updatemask << 1;
		}
	}

	template <typename T, typename S> 
	S SAMKeyEntry<T, S>::getSETStruct()
	{
		S set;
		if (d_samType == SAMType::SAM_AV1)
			set = new SETAV1;
		else
			set = new SETAV2;

		char *x = (char*)set;
		
		unsigned char set_save[2];
		unsigned char j = 0;

		memcpy(set_save, d_keyentryinformation.set, 2);

		for (char i = 7; i >= 0; --i)
		{
			if ((set_save[j] & 0x80) == 0x80)
				x[i + j * 8] = 1;
			else
				x[i + j * 8] = 0;
			set_save[j] = set_save[j] << 1;
			if (i == 0 && j == 0)
			{
				i = 8;
				j++;
			}
		}

		return set;
	}

	template <typename T, typename S> 
	void SAMKeyEntry<T, S>::setSET(const S& t)
	{
		char *x = (char*)&t;
		memset(d_keyentryinformation.set, 0, 2);
		unsigned char j = 0;
		for (char i = 7; i >= 0; --i)
		{
			d_keyentryinformation.set[j] += (char)x[i + j * 8];
			if (i == 0 && j == 0)
			{
				i = 8;
				j++;
			}
			else if (i != 0) 
				d_keyentryinformation.set[j] = d_keyentryinformation.set[j] << 1;
		}
	}
}
