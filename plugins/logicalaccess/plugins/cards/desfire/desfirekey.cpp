/**
 * \file desfirekey.cpp
 * \author Maxime C. <maxime@leosac.com>
 * \brief DESFire Key.
 */

#include <boost/property_tree/ptree.hpp>
#include <logicalaccess/plugins/cards/desfire/desfirekey.hpp>

namespace logicalaccess
{
DESFireKey::DESFireKey()
    : Key()
{
    d_keyType = DF_KEY_DES;
    d_key_version = 0;
    d_length = 0;
    Key::clear();
}

DESFireKey::DESFireKey(const std::string &str)
    : Key()
{
    d_keyType = DF_KEY_DES;
    d_key_version = 0;
    d_length = 0;
    Key::fromString(str);
}

DESFireKey::DESFireKey(const void *buf, size_t buflen)
    : Key()
{
    d_keyType = DF_KEY_DES;
    d_key_version = 0;
    d_length = 0;
    Key::setData(buf, buflen);
}

DESFireKey::DESFireKey(const ByteVector &data)
    : Key()
{
    d_keyType = DF_KEY_DES;
    d_key_version = 0;
    d_length = 0;
    Key::setData(data);
}

size_t DESFireKey::getLength() const
{
    if (d_length > 0)
    {
        return d_length;
    }
    
    size_t length = 0;

    switch (d_keyType)
    {
    case DF_KEY_DES: length = DESFIRE_DES_KEY_SIZE; break;

    case DF_KEY_3K3DES: length = DESFIRE_MAXKEY_SIZE; break;

    case DF_KEY_AES: length = DESFIRE_AES_KEY_SIZE; break;
    }

    return length;
}

void DESFireKey::serialize(boost::property_tree::ptree &parentNode)
{
    boost::property_tree::ptree node;
    Key::serialize(node);

    node.put("KeyType", d_keyType);
    node.put("KeyVersion", d_key_version);

    parentNode.add_child(getDefaultXmlNodeName(), node);
}

void DESFireKey::unSerialize(boost::property_tree::ptree &node)
{
    Key::unSerialize(node);
    d_keyType =
        static_cast<DESFireKeyType>(node.get_child("KeyType").get_value<unsigned int>());
    d_key_version = node.get_child("KeyVersion").get_value<unsigned char>();
}

std::string DESFireKey::getDefaultXmlNodeName() const
{
    return "DESFireKey";
}

bool DESFireKey::operator==(const DESFireKey &key) const
{
    if (!Key::operator==(key))
    {
        return false;
    }

    return (getKeyVersion() == key.getKeyVersion() && getKeyType() == key.getKeyType());
}

std::string DESFireKey::DESFireKeyTypeStr(DESFireKeyType t)
{
    switch (t)
    {
    case DF_KEY_DES: return "DF_KEY_DES";
    case DF_KEY_3K3DES: return "DF_KEY_3K3DES";
    case DF_KEY_AES: return "DF_KEY_AES";
    default: return "Unknown";
    }
}

void DESFireKey::setKeyType(DESFireKeyType keyType)
{
    d_keyType = keyType;
    d_data.resize(getLength(), 0x00);
}
}