/**
 * \file mifarekey.cpp
 * \author Maxime C. <maxime@leosac.com>
 * \brief Mifare Key.
 */

#include <cstring>
#include <boost/property_tree/ptree.hpp>
#include <logicalaccess/plugins/cards/mifare/mifarekey.hpp>

namespace logicalaccess
{
MifareKey::MifareKey()
    : Key()
{
    Key::clear();
}

MifareKey::MifareKey(const std::string &str)
    : Key()
{
    Key::fromString(str);
}

MifareKey::MifareKey(const void *buf, size_t buflen)
    : Key()
{
    Key::setData(buf, buflen);
}

MifareKey::MifareKey(const ByteVector &data)
    : Key()
{
    Key::setData(data);
}

void MifareKey::serialize(boost::property_tree::ptree &parentNode)
{
    boost::property_tree::ptree node;
    Key::serialize(node);
    parentNode.add_child(getDefaultXmlNodeName(), node);
}

void MifareKey::unSerialize(boost::property_tree::ptree &node)
{
    Key::unSerialize(node);
}

std::string MifareKey::getDefaultXmlNodeName() const
{
    return "MifareKey";
}
}