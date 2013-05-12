/**
 * \file smartframechip.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief SmartFrame chip.
 */

#include "smartframechip.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>

namespace logicalaccess
{
	SmartFrameChip::SmartFrameChip()
		: Chip(CHIP_SMARTFRAME)
	{
	}

	SmartFrameChip::~SmartFrameChip()
	{
	}

	boost::shared_ptr<LocationNode> SmartFrameChip::getRootLocationNode()
	{
		boost::shared_ptr<LocationNode> rootNode;
		rootNode.reset(new LocationNode());

		rootNode->setName("SmartFrame");

		return rootNode;
	}
}
