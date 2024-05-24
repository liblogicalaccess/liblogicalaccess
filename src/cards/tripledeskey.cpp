/**
 * \file TripleDESKey.cpp
 * \author Maxime C. <maxime@leosac.com>
 * \brief Triple DES Key.
 */

#include <logicalaccess/cards/tripledeskey.hpp>
#include <cstring>
#include <boost/property_tree/ptree.hpp>

namespace logicalaccess
{
TripleDESKey::TripleDESKey()
    : Key()
{
    Key::clear();
}

TripleDESKey::TripleDESKey(const std::string &str)
    : Key()
{
    Key::fromString(str);
}

TripleDESKey::TripleDESKey(const void *buf, size_t buflen)
    : Key()
{
    Key::setData(buf, buflen);
}

TripleDESKey::TripleDESKey(const ByteVector &data)
    : Key()
{
    Key::setData(data);
}

void TripleDESKey::serialize(boost::property_tree::ptree &parentNode)
{
    boost::property_tree::ptree node;
    Key::serialize(node);
    parentNode.add_child(getDefaultXmlNodeName(), node);
}

void TripleDESKey::unSerialize(boost::property_tree::ptree &node)
{
    Key::unSerialize(node);
}

std::string TripleDESKey::getDefaultXmlNodeName() const
{
    return "TripleDESKey";
}
}