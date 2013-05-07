/**
 * \file EM4135Chip.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief EM4135 chip.
 */

#include "EM4135Chip.h"

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
	EM4135Chip::EM4135Chip()
		: Chip("EM4135")
	{
	}

	EM4135Chip::~EM4135Chip()
	{
	}

	boost::shared_ptr<LocationNode> EM4135Chip::getRootLocationNode()
	{
		boost::shared_ptr<LocationNode> rootNode;
		rootNode.reset(new LocationNode());

		rootNode->setName("EM4135");

		return rootNode;
	}
}
