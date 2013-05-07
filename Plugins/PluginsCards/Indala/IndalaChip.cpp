/**
 * \file IndalaChip.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Indala chip.
 */

#include "IndalaChip.h"

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
	IndalaChip::IndalaChip()
		: Chip(CHIP_INDALA)
	{
	}

	IndalaChip::~IndalaChip()
	{
	}

	boost::shared_ptr<LocationNode> IndalaChip::getRootLocationNode()
	{
		boost::shared_ptr<LocationNode> rootNode;
		rootNode.reset(new LocationNode());

		rootNode->setName("Indala");

		return rootNode;
	}
}
