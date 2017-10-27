/**
 * \file cps3location.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Location informations for CPS3 card.
 */

#include <boost/property_tree/ptree.hpp>
#include "cps3location.hpp"

namespace logicalaccess
{
CPS3Location::CPS3Location()
{
}

CPS3Location::~CPS3Location()
{
}

void CPS3Location::serialize(boost::property_tree::ptree &parentNode)
{
    boost::property_tree::ptree node;

    parentNode.add_child(getDefaultXmlNodeName(), node);
}

void CPS3Location::unSerialize(boost::property_tree::ptree & /*node*/)
{
}

std::string CPS3Location::getDefaultXmlNodeName() const
{
    return "CPS3Location";
}

bool CPS3Location::operator==(const Location &location) const
{
    if (!Location::operator==(location))
        return false;

    if (typeid(location) != typeid(CPS3Location))
        return false;

    const CPS3Location *pxLocation = dynamic_cast<const CPS3Location *>(&location);

    if (!pxLocation)
        return false;

    return (true);
}
}