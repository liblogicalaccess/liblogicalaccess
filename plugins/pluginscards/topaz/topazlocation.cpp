/**
 * \file topazlocation.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Location informations for Topaz card.
 */

#include <boost/property_tree/ptree.hpp>
#include "topazlocation.hpp"

namespace logicalaccess
{
    TopazLocation::TopazLocation() :
        page(1),
		byte_(0)
    {
    }

    TopazLocation::~TopazLocation()
    {
    }

    void TopazLocation::serialize(boost::property_tree::ptree& parentNode)
    {
        boost::property_tree::ptree node;

        node.put("Page", page);
        node.put("Byte", byte_);

        parentNode.add_child(getDefaultXmlNodeName(), node);
    }

    void TopazLocation::unSerialize(boost::property_tree::ptree& node)
    {
        page = node.get_child("Page").get_value<int>();
		byte_ = node.get_child("Byte").get_value<int>();
    }

    std::string TopazLocation::getDefaultXmlNodeName() const
    {
        return "TopazLocation";
    }

    bool TopazLocation::operator==(const Location& location) const
    {
        if (!Location::operator==(location))
            return false;

        if (typeid(location) != typeid(TopazLocation))
            return false;

        const TopazLocation* mLocation = dynamic_cast<const TopazLocation*>(&location);

        if (!mLocation)
            return false;

        return (page == mLocation->page
            && byte_ == mLocation->byte_);
    }
}