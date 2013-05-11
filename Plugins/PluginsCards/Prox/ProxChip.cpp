/**
 * \file ProxChip.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Prox chip.
 */

#include "ProxChip.h"
#include "ProxProfile.h"

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
	ProxChip::ProxChip()
		: Chip("Prox")
	{
		d_profile.reset(new ProxProfile());
	}

	ProxChip::~ProxChip()
	{
	}	

	boost::shared_ptr<LocationNode> ProxChip::getRootLocationNode()
	{
		boost::shared_ptr<LocationNode> rootNode;
		rootNode.reset(new LocationNode());

		rootNode->setName("HID Prox");

		return rootNode;
	}
}
