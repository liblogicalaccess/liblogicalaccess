/**
 * \file em4102chip.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief EM4102 chip.
 */

#include "em4102chip.hpp"

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

#include "logicalaccess/services/accesscontrol/formats/bithelper.hpp"

namespace logicalaccess
{
	EM4102Chip::EM4102Chip()
		: Chip(CHIP_EM4102)
	{
	}

	EM4102Chip::~EM4102Chip()
	{
	}

	boost::shared_ptr<LocationNode> EM4102Chip::getRootLocationNode()
	{
		boost::shared_ptr<LocationNode> rootNode;
		rootNode.reset(new LocationNode());

		rootNode->setName("EM4102");

		return rootNode;
	}
}
