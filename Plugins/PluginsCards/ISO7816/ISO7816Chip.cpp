/**
 * \file ISO7816Chip.cpp
 * \author Maxime CHAMLEY <maxime.chamley@islog.eu>
 * \brief ISO7816 chip.
 */

#include "ISO7816Chip.h"
#include "ISO7816Profile.h"

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


namespace LOGICALACCESS
{
	ISO7816Chip::ISO7816Chip(std::string ct)
		: Chip(ct)
	{
		d_profile.reset(new ISO7816Profile());
	}

	ISO7816Chip::ISO7816Chip()
		: Chip(CHIP_ISO7816)
	{
		d_profile.reset(new ISO7816Profile());
	}

	ISO7816Chip::~ISO7816Chip()
	{
	}	

	boost::shared_ptr<LocationNode> ISO7816Chip::getRootLocationNode()
	{
		boost::shared_ptr<LocationNode> rootNode;
		rootNode.reset(new LocationNode());
		rootNode->setName("ISO 7816");		

		return rootNode;
	}
}
