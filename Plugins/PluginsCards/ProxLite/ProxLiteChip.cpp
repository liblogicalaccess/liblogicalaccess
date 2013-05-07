/**
 * \file ProxLiteChip.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief ProxLite chip.
 */

#include "ProxLiteChip.h"

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
	ProxLiteChip::ProxLiteChip()
		: Chip(CHIP_PROXLITE)
	{
	}

	ProxLiteChip::~ProxLiteChip()
	{
	}

	boost::shared_ptr<LocationNode> ProxLiteChip::getRootLocationNode()
	{
		boost::shared_ptr<LocationNode> rootNode;
		rootNode.reset(new LocationNode());

		rootNode->setName("ProxLite");

		return rootNode;
	}
}
