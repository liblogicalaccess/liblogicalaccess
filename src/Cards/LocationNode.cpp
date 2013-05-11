/**
 * \file LocationNode.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Location node.
 */

#include "logicalaccess/Cards/LocationNode.h"
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

	string LocationNode::getName()
	{
		return d_name;
	}

	vector<boost::shared_ptr<LocationNode> >& LocationNode::getChildrens()
	{
		return d_childrens;
	}

	void LocationNode::setNeedAuthentication(bool needed)
	{
		d_needAuthentication = needed;
	}

	bool LocationNode::getNeedAuthentication()
	{
		return d_needAuthentication;
	}

	void LocationNode::setLength(size_t length)
	{
		d_length = length;
	}

	size_t LocationNode::getLength()
	{
		return d_length;
	}

	void LocationNode::setLocation(boost::shared_ptr<Location> location)
	{
		d_location = location;
	}

	boost::shared_ptr<Location> LocationNode::getLocation()
	{
		return d_location;
	}

	void LocationNode::setUnit(size_t unit)
	{
		d_unit = unit;
	}

	size_t LocationNode::getUnit()
	{
		return d_unit;
	}

	void LocationNode::setHasProperties(bool hasProperties)
	{
		d_hasProperties = hasProperties;
	}

	bool LocationNode::getHasProperties()
	{
		return d_hasProperties;
	}

	void LocationNode::setParent(boost::weak_ptr<LocationNode> parent)
	{
		d_parent = parent;
	}

	boost::shared_ptr<LocationNode> LocationNode::getParent()
	{
		return d_parent.lock();
	}
}

