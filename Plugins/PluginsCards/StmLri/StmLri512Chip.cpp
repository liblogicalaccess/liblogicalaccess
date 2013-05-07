/**
 * \file StmLri512Chip.cpp
 * \author Maxime CHAMLEY <maxime.chamley@islog.eu>
 * \brief StmLri512 chip.
 */

#include "StmLri512Chip.h"

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
	StmLri512Chip::StmLri512Chip()
		: Chip(CHIP_STMLRI512)
	{
	}

	StmLri512Chip::~StmLri512Chip()
	{
	}

	boost::shared_ptr<LocationNode> StmLri512Chip::getRootLocationNode()
	{
		boost::shared_ptr<LocationNode> rootNode;
		rootNode.reset(new LocationNode());

		rootNode->setName("STM LRI512");

		return rootNode;
	}
}
