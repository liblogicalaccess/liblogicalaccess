/**
 * \file InfineonMYDChip.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Infineon My-D chip.
 */

#include "InfineonMYDChip.h"

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
	InfineonMYDChip::InfineonMYDChip()
		: ISO15693Chip(CHIP_INFINEONMYD)
	{
	}

	InfineonMYDChip::~InfineonMYDChip()
	{
	}

	boost::shared_ptr<LocationNode> InfineonMYDChip::getRootLocationNode()
	{
		boost::shared_ptr<LocationNode> rootNode;
		rootNode.reset(new LocationNode());

		rootNode->setName("Infineon My-D");

		return rootNode;
	}
}
