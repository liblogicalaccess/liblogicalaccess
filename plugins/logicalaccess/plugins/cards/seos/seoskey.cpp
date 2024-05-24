/**
 * \file seoskey.cpp
 * \author Maxime C. <maxime@leosac.com>
 * \brief SEOS Key.
 */

#include <boost/property_tree/ptree.hpp>
#include <logicalaccess/plugins/cards/seos/seoskey.hpp>

namespace logicalaccess
{
SeosKey::SeosKey()
    : Key()
{
    d_keyType = SEOS_KEY_DES;
    Key::clear();
}

SeosKey::SeosKey(const std::string &str)
    : Key()
{
    d_keyType = SEOS_KEY_DES;
    Key::fromString(str);
}

SeosKey::SeosKey(const void *buf, size_t buflen)
    : Key()
{
    d_keyType = SEOS_KEY_DES;
    Key::setData(buf, buflen);
}

SeosKey::SeosKey(const ByteVector &data)
    : Key()
{
    Key::setData(data);
}

size_t SeosKey::getLength() const
{
    return SEOS_KEY_SIZE;
}

void SeosKey::serialize(boost::property_tree::ptree &parentNode)
{
    boost::property_tree::ptree node;
    Key::serialize(node);

    node.put("KeyType", d_keyType);

    parentNode.add_child(getDefaultXmlNodeName(), node);
}

void SeosKey::unSerialize(boost::property_tree::ptree &node)
{
    Key::unSerialize(node);
    d_keyType =
        static_cast<SeosKeyType>(node.get_child("KeyType").get_value<unsigned int>());
}

std::string SeosKey::getDefaultXmlNodeName() const
{
    return "SeosKey";
}

bool SeosKey::operator==(const SeosKey &key) const
{
    if (!Key::operator==(key))
    {
        return false;
    }

    return (getKeyType() == key.getKeyType());
}

std::string SeosKey::SeosKeyTypeStr(SeosKeyType t)
{
    switch (t)
    {
    case SEOS_KEY_DES: return "SEOS_KEY_DES";
    case SEOS_KEY_AES: return "SEOS_KEY_AES";
    default: return "Unknown";
    }
}

void SeosKey::setKeyType(SeosKeyType keyType)
{
    d_keyType = keyType;
    d_data.resize(getLength(), 0x00);
}
}