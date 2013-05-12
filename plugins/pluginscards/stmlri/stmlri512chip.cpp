/**
 * \file stmlri512chip.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief StmLri512 chip.
 */

#include "stmlri512chip.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>

namespace logicalaccess
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
