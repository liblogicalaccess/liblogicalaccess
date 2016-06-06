/**
 * \file topazaccessinfo.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Access informations for Topaz card.
 */

#include <boost/property_tree/ptree.hpp>
#include "topazaccessinfo.hpp"
#include "topazchip.hpp"

namespace logicalaccess
{
    TopazAccessInfo::TopazAccessInfo()
    {
        lockPage = false;
    }

    TopazAccessInfo::~TopazAccessInfo()
    {
    }

    std::string TopazAccessInfo::getCardType() const
    {
        return CHIP_TOPAZ;
    }

    void TopazAccessInfo::generateInfos()
    {
    }

    void TopazAccessInfo::serialize(boost::property_tree::ptree& parentNode)
    {
        boost::property_tree::ptree node;
        node.put("LockPage", lockPage);
        parentNode.add_child(TopazAccessInfo::getDefaultXmlNodeName(), node);
    }

    void TopazAccessInfo::unSerialize(boost::property_tree::ptree& node)
    {
        lockPage = node.get_child("LockPage").get_value<bool>();
    }

    std::string TopazAccessInfo::getDefaultXmlNodeName() const
    {
        return "TopazAccessInfo";
    }

    bool TopazAccessInfo::operator==(const AccessInfo& ai) const
    {
        if (!AccessInfo::operator==(ai))
            return false;

        if (typeid(ai) != typeid(AccessInfo))
            return false;

        const TopazAccessInfo* mAi = dynamic_cast<const TopazAccessInfo*>(&ai);

        if (!mAi)
            return false;

        return (lockPage == mAi->lockPage);
    }
}