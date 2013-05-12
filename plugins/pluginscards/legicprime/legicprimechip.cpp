/**
 * \file legicprimechip.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Legic Prime chip.
 */

#include "legicprimechip.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>

namespace logicalaccess
{
	LegicPrimeChip::LegicPrimeChip()
		: Chip(CHIP_LEGICPRIME)
	{
	}

	LegicPrimeChip::~LegicPrimeChip()
	{
	}

	boost::shared_ptr<LocationNode> LegicPrimeChip::getRootLocationNode()
	{
		boost::shared_ptr<LocationNode> rootNode;
		rootNode.reset(new LocationNode());

		rootNode->setName("Legic Prime");

		return rootNode;
	}
}
