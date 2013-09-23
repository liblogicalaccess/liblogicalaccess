/**
 * \file desfirekey.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief DESFire Key.
 */

#include "desfirekey.hpp"

namespace logicalaccess
{
	DESFireKey::DESFireKey()
		: Key()
	{
		d_keyType = DF_KEY_DES;
		d_key.resize(getLength());
		d_key_version = 0;
		clear();
	}

	DESFireKey::DESFireKey(const std::string& str)
		: Key()
	{
		d_keyType = DF_KEY_DES;
		d_key.resize(getLength());
		d_key_version = 0;	
		clear();
		fromString(str);
	}

	DESFireKey::DESFireKey(const void* buf, size_t buflen)
		: Key()
	{
		d_keyType = DF_KEY_DES;
		d_key.resize(getLength());
		d_key_version = 0;
		clear();

		if (buf != NULL)
		{
			if (buflen >= getLength())
			{
				d_key.clear();
				d_key.insert(d_key.end(), reinterpret_cast<const unsigned char*>(buf), reinterpret_cast<const unsigned char*>(buf) + getLength());
				d_isEmpty = false;
			}
		}
	}

	size_t DESFireKey::getLength() const
	{
		size_t length = 0;

		switch (d_keyType)
		{
		case DF_KEY_DES:
			length = DESFIRE_DES_KEY_SIZE;
			break;

		case DF_KEY_3K3DES:
			length = DESFIRE_MAXKEY_SIZE;
			break;

		case DF_KEY_AES:
			length = DESFIRE_AES_KEY_SIZE;
			break;
		}

		return length;
	}

	void DESFireKey::serialize(boost::property_tree::ptree& parentNode)
	{
		boost::property_tree::ptree node;
		Key::serialize(node);

		node.put("KeyType", d_keyType);
		node.put("KeyVersion", d_key_version);

		parentNode.add_child(getDefaultXmlNodeName(), node);
	}

	void DESFireKey::unSerialize(boost::property_tree::ptree& node)
	{
		Key::unSerialize(node);
		d_keyType = static_cast<DESFireKeyType>(node.get_child("KeyType").get_value<unsigned int>());
		d_key_version = node.get_child("KeyVersion").get_value<unsigned char>();
	}

	std::string DESFireKey::getDefaultXmlNodeName() const
	{
		return "DESFireKey";
	}

	bool DESFireKey::operator==(const DESFireKey& key) const
	{
		if (!Key::operator==(key))
		{
			return false;
		}
		if (isEmpty() && key.isEmpty())
		{
			return true;
		}

		if (getLength() != key.getLength() || isEmpty() || key.isEmpty())
		{
			return false;
		}

		return (getKeyVersion() == key.getKeyVersion() && getKeyType() == key.getKeyType());
	}

	std::string DESFireKey::DESFireKeyTypeStr(DESFireKeyType t)
	{
		switch (t) {
			case DF_KEY_DES: return "DF_KEY_DES";
			case DF_KEY_3K3DES: return "DF_KEY_3K3DES";
			case DF_KEY_AES: return "DF_KEY_AES";
			default: return "Unknown";
		}
	}
}