/**
 * \file ICode2Chip.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief iCode2 chip.
 */

#include "ICode2Chip.h"

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
	ICode2Chip::ICode2Chip()
		: ISO15693Chip(CHIP_ICODE2)
	{
	}

	ICode2Chip::~ICode2Chip()
	{
	}

	boost::shared_ptr<LocationNode> ICode2Chip::getRootLocationNode()
	{
		boost::shared_ptr<LocationNode> rootNode;
		rootNode.reset(new LocationNode());

		rootNode->setName("iCode2");

		return rootNode;
	}
}
