/**
 * \file proxchip.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Prox chip.
 */

#include "proxchip.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>

namespace logicalaccess
{
	ProxChip::ProxChip()
		: Chip("Prox")
	{
		d_profile.reset(new ProxProfile());
	}

	ProxChip::~ProxChip()
	{
	}	

	boost::shared_ptr<LocationNode> ProxChip::getRootLocationNode()
	{
		boost::shared_ptr<LocationNode> rootNode;
		rootNode.reset(new LocationNode());

		rootNode->setName("HID Prox");

		return rootNode;
	}
}
