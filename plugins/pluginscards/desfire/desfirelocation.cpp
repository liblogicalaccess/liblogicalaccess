/**
 * \file desfirelocation.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Location informations for DESFire card.
 */

#include <boost/property_tree/ptree.hpp>
#include "desfirelocation.hpp"

namespace logicalaccess
{
DESFireLocation::DESFireLocation()
{
    aid           = 0x000521;
    file          = 0;
    byte_         = 0;
    securityLevel = CM_ENCRYPT;
}

DESFireLocation::~DESFireLocation()
{
}

unsigned int DESFireLocation::convertAidToUInt(const ByteVector &aid)
{
    // LSB first
    int ret = ((aid[0] & 0xff) | ((((aid[1] & 0xff) << 8) & 0xff00) |
                                  (((aid[2] & 0xff) << 16) & 0xff0000)));

    return ret;
}

void DESFireLocation::convertUIntToAid(unsigned int i, ByteVector &aid)
{
    // LSB first
    aid.push_back(i & 0xff);
    aid.push_back((i >> 8) & 0xff);
    aid.push_back((i >> 16) & 0xff);
}

void DESFireLocation::serialize(boost::property_tree::ptree &parentNode)
{
    boost::property_tree::ptree node;

    node.put("AID", aid);
    node.put("File", file);
    node.put("Byte", byte_);
    node.put("SecurityLevel", securityLevel);

    parentNode.add_child(DESFireLocation::getDefaultXmlNodeName(), node);
}

void DESFireLocation::unSerialize(boost::property_tree::ptree &node)
{
    aid           = node.get_child("AID").get_value<int>();
    file          = node.get_child("File").get_value<int>();
    byte_         = node.get_child("Byte").get_value<int>();
    securityLevel = static_cast<EncryptionMode>(
        node.get_child("SecurityLevel").get_value<unsigned int>());
}

std::string DESFireLocation::getDefaultXmlNodeName() const
{
    return "DESFireLocation";
}

bool DESFireLocation::operator==(const Location &location) const
{
    if (!Location::operator==(location))
        return false;

    if (typeid(location) != typeid(DESFireLocation))
        return false;

    const DESFireLocation *dfLocation = dynamic_cast<const DESFireLocation *>(&location);

    if (!dfLocation)
        return false;

    return (aid == dfLocation->aid && file == dfLocation->file &&
            byte_ == dfLocation->byte_ && securityLevel == dfLocation->securityLevel);
}
}