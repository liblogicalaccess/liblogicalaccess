/**
 * \file ICode1Chip.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief iCode1 chip.
 */

#include "ICode1Chip.h"

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

namespace logicalaccess
{
	ICode1Chip::ICode1Chip()
		: ISO15693Chip(CHIP_ICODE1)
	{
	}

	ICode1Chip::~ICode1Chip()
	{
	}

	boost::shared_ptr<LocationNode> ICode1Chip::getRootLocationNode()
	{
		boost::shared_ptr<LocationNode> rootNode;
		rootNode.reset(new LocationNode());

		rootNode->setName("iCode1");

		return rootNode;
	}
}
