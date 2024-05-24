/**
 * \file hmac1key.cpp
 * \author Maxime C. <maxime@leosac.com>
 * \brief HMAC1 Key.
 */

#include <logicalaccess/cards/hmac1key.hpp>
#include <cstring>
#include <boost/property_tree/ptree.hpp>

namespace logicalaccess
{
HMAC1Key::HMAC1Key()
    : Key()
{
    Key::clear();
}

HMAC1Key::HMAC1Key(const std::string &str)
    : Key()
{
    Key::fromString(str);
}

HMAC1Key::HMAC1Key(const void *buf, size_t buflen)
    : Key()
{
    Key::setData(buf, buflen);
}

HMAC1Key::HMAC1Key(const ByteVector &data)
    : Key()
{
    Key::setData(data);
}

void HMAC1Key::serialize(boost::property_tree::ptree &parentNode)
{
    boost::property_tree::ptree node;
    Key::serialize(node);
    parentNode.add_child(getDefaultXmlNodeName(), node);
}

void HMAC1Key::unSerialize(boost::property_tree::ptree &node)
{
    Key::unSerialize(node);
}

std::string HMAC1Key::getDefaultXmlNodeName() const
{
    return "HMAC1Key";
}
}