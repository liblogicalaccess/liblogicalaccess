/**
 * \file SmartFrameChip.cpp
 * \author Maxime CHAMLEY <maxime.chamley@islog.eu>
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
		: Chip("SmartFrame")
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
