/**
 * \file generictagchip.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Generic tag chip.
 */

#include "generictagchip.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>

namespace logicalaccess
{
	GenericTagChip::GenericTagChip()
		: Chip(CHIP_GENERICTAG)
	{
		d_tagIdBitsLength = 0;
	}

	GenericTagChip::~GenericTagChip()
	{
	}

	boost::shared_ptr<LocationNode> GenericTagChip::getRootLocationNode()
	{
		boost::shared_ptr<LocationNode> rootNode;
		rootNode.reset(new LocationNode());

		rootNode->setName("RFID Tag");

		return rootNode;
	}
}
