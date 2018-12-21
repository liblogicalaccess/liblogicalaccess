/**
 * \file seoskey.cpp
 * \author Maxime C. <maxime-dev@islog.com>
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
    d_key.resize(SeosKey::getLength());
    Key::clear();
}

SeosKey::SeosKey(const std::string &str)
    : Key()
{
    d_keyType = SEOS_KEY_DES;
    d_key.resize(SeosKey::getLength());
    Key::clear();
    Key::fromString(str);
}

SeosKey::SeosKey(const void *buf, size_t buflen)
    : Key()
{
    d_keyType = SEOS_KEY_DES;
    d_key.resize(SeosKey::getLength());
    Key::clear();

    if (buf != nullptr && buflen >= SeosKey::getLength())
    {
        d_key.clear();
        d_key.insert(d_key.end(), reinterpret_cast<const unsigned char *>(buf),
                     reinterpret_cast<const unsigned char *>(buf) + SeosKey::getLength());
        d_isEmpty = false;
    }
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
    if (isEmpty() && key.isEmpty())
    {
        return true;
    }

    if (getLength() != key.getLength() || isEmpty() || key.isEmpty())
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
    d_key.resize(getLength());
}
}