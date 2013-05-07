/**
 * \file SmartFrameChip.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief SmartFrame chip.
 */

#include "SmartFrameChip.h"

#include <iostream>
#include <iomanip>
#include <sstream>

using std::endl;
using std::dec;
using std::hex;
using std::setfill;
using std::setw;
using std::ostringstream;
using std::istringstream;

#include "logicalaccess/Services/AccessControl/Formats/BitHelper.h"

namespace LOGICALACCESS
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
