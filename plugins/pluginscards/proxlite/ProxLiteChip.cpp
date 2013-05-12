/**
 * \file proxlitechip.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief ProxLite chip.
 */

#include "proxlitechip.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>

namespace logicalaccess
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
