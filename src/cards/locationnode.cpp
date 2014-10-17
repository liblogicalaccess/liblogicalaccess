/**
 * \file locationnode.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Location node.
 */

#include "logicalaccess/cards/locationnode.hpp"
#include <stdlib.h>

using std::string;
using std::vector;
using std::ostream;

namespace logicalaccess
{
    LocationNode::LocationNode()
        : d_needAuthentication(false), d_hasProperties(false), d_length(0), d_unit(1)
    {
    }

    LocationNode::~LocationNode()
    {
    }

    void LocationNode::setName(const string& name)
    {
        d_name = name;
    }

    string LocationNode::getName() const
    {
        return d_name;
    }

    vector<std::shared_ptr<LocationNode> >& LocationNode::getChildrens()
    {
        return d_childrens;
    }

    void LocationNode::setNeedAuthentication(bool needed)
    {
        d_needAuthentication = needed;
    }

    bool LocationNode::getNeedAuthentication() const
    {
        return d_needAuthentication;
    }

    void LocationNode::setLength(size_t length)
    {
        d_length = length;
    }

    size_t LocationNode::getLength() const
    {
        return d_length;
    }

    void LocationNode::setLocation(std::shared_ptr<Location> location)
    {
        d_location = location;
    }

    std::shared_ptr<Location> LocationNode::getLocation() const
    {
        return d_location;
    }

    void LocationNode::setUnit(size_t unit)
    {
        d_unit = unit;
    }

    size_t LocationNode::getUnit() const
    {
        return d_unit;
    }

    void LocationNode::setHasProperties(bool hasProperties)
    {
        d_hasProperties = hasProperties;
    }

    bool LocationNode::getHasProperties() const
    {
        return d_hasProperties;
    }

    void LocationNode::setParent(std::weak_ptr<LocationNode> parent)
    {
        d_parent = parent;
    }

    std::shared_ptr<LocationNode> LocationNode::getParent()
    {
        return d_parent.lock();
    }
}