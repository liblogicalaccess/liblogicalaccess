/**
 * \file EM4102Chip.cpp
 * \author Maxime CHAMLEY <maxime.chamley@islog.eu>
 * \brief EM4102 chip.
 */

#include "EM4102Chip.h"

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
