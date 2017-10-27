/**
 * \file locationnode.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Location node.
 */

#include "logicalaccess/cards/locationnode.hpp"
#include <stdlib.h>

namespace logicalaccess
{
LocationNode::LocationNode()
    : d_needAuthentication(false)
    , d_hasProperties(false)
    , d_length(0)
    , d_unit(1)
    , d_can_read(true)
    , d_can_write(true)
{
}

LocationNode::~LocationNode()
{
}

void LocationNode::setName(const std::string &name)
{
    d_name = name;
}

std::string LocationNode::getName() const
{
    return d_name;
}

std::vector<std::shared_ptr<LocationNode>> &LocationNode::getChildrens()
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

std::shared_ptr<LocationNode> LocationNode::getParent() const
{
    return d_parent.lock();
}

void LocationNode::setCanRead(bool can)
{
    d_can_read = can;
}

bool LocationNode::getCanRead() const
{
    return d_can_read;
}

void LocationNode::setCanWrite(bool can)
{
    d_can_write = can;
}

bool LocationNode::getCanWrite() const
{
    return d_can_write;
}
}