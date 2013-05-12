/**
 * \file tagitchip.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Tag-It chip.
 */

#include "tagitchip.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>

namespace logicalaccess
{
	TagItChip::TagItChip()
		: ISO15693Chip(CHIP_TAGIT)
	{

	}

	TagItChip::~TagItChip()
	{

	}	

	boost::shared_ptr<LocationNode> TagItChip::getRootLocationNode()
	{
		boost::shared_ptr<LocationNode> rootNode = ISO15693Chip::getRootLocationNode();
		rootNode->setName("Tag-It");

		return rootNode;
	}
}