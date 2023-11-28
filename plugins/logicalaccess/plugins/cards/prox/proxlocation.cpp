/**
 * \file proxlocation.cpp
 * \author Maxime C. <maxime@leosac.com>
 * \brief Location informations for Prox card.
 */

#include <boost/property_tree/ptree.hpp>
#include <logicalaccess/plugins/cards/prox/proxlocation.hpp>

namespace logicalaccess
{
ProxLocation::ProxLocation()
{
    bit = 0;
}

ProxLocation::~ProxLocation()
{
}

void ProxLocation::serialize(boost::property_tree::ptree &parentNode)
{
    boost::property_tree::ptree node;

    node.put("Bit", bit);

    parentNode.add_child(getDefaultXmlNodeName(), node);
}

void ProxLocation::unSerialize(boost::property_tree::ptree &node)
{
    bit = node.get_child("Bit").get_value<int>();
}

std::string ProxLocation::getDefaultXmlNodeName() const
{
    return "ProxLocation";
}

bool ProxLocation::operator==(const Location &location) const
{
    if (!Location::operator==(location))
        return false;

    if (typeid(location) != typeid(ProxLocation))
        return false;

    const ProxLocation *pxLocation = dynamic_cast<const ProxLocation *>(&location);

    if (!pxLocation)
        return false;

    return (bit == pxLocation->bit);
}
}