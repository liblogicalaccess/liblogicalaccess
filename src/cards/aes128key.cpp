/**
 * \file aes128key.cpp
 * \author Maxime C. <maxime@leosac.com>
 * \brief AES128 Key.
 */

#include <logicalaccess/cards/aes128key.hpp>
#include <cstring>
#include <boost/property_tree/ptree.hpp>

namespace logicalaccess
{
AES128Key::AES128Key()
    : Key()
{
    Key::clear();
}

AES128Key::AES128Key(const std::string &str)
    : Key()
{
    Key::fromString(str);
}

AES128Key::AES128Key(const void *buf, size_t buflen)
    : Key()
{
    Key::setData(buf, buflen);
}

AES128Key::AES128Key(const ByteVector &data)
    : Key()
{
    Key::setData(data);
}

void AES128Key::serialize(boost::property_tree::ptree &parentNode)
{
    boost::property_tree::ptree node;
    Key::serialize(node);
    parentNode.add_child(getDefaultXmlNodeName(), node);
}

void AES128Key::unSerialize(boost::property_tree::ptree &node)
{
    Key::unSerialize(node);
}

std::string AES128Key::getDefaultXmlNodeName() const
{
    return "AES128Key";
}
}