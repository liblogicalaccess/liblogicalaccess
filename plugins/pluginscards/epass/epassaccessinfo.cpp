#include "epassaccessinfo.hpp"
#include <boost/property_tree/ptree.hpp>

using namespace logicalaccess;


void EPassAccessInfo::generateInfos()
{
    // wtf is this?
}

std::string EPassAccessInfo::getCardType() const
{
    return "EPass";
}

void EPassAccessInfo::serialize(boost::property_tree::ptree &parentNode)
{
    boost::property_tree::ptree node;

    node.add("mrz", mrz_);
    parentNode.add_child(getDefaultXmlNodeName(), node);
}

void EPassAccessInfo::unSerialize(boost::property_tree::ptree &node)
{
    mrz_ = node.get<std::string>("mrz", "");
}

std::string EPassAccessInfo::getDefaultXmlNodeName() const
{
    return "EPassAccessInfo";
}
