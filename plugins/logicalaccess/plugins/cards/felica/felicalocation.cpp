/**
 * \file felicalocation.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Location informations for FeliCa card.
 */

#include <boost/property_tree/ptree.hpp>
#include <logicalaccess/plugins/cards/felica/felicalocation.hpp>

namespace logicalaccess
{
FeliCaLocation::FeliCaLocation()
{
    code  = 0;
    block = 0;
}

FeliCaLocation::~FeliCaLocation()
{
}

void FeliCaLocation::serialize(boost::property_tree::ptree &parentNode)
{
    boost::property_tree::ptree node;

    node.put("Code", code);
    node.put("Block", block);

    parentNode.add_child(getDefaultXmlNodeName(), node);
}

void FeliCaLocation::unSerialize(boost::property_tree::ptree &node)
{
    code  = node.get_child("Code").get_value<unsigned short>();
    block = node.get_child("Block").get_value<unsigned short>();
}

std::string FeliCaLocation::getDefaultXmlNodeName() const
{
    return "FeliCaLocation";
}

bool FeliCaLocation::operator==(const Location &location) const
{
    if (!Location::operator==(location))
        return false;

    if (typeid(location) != typeid(FeliCaLocation))
        return false;

    const FeliCaLocation *pxLocation = dynamic_cast<const FeliCaLocation *>(&location);

    if (!pxLocation)
        return false;

    return (code == pxLocation->code && block == pxLocation->block);
}
}