/**
 * \file iso7816chip.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief ISO7816 chip.
 */

#include "iso7816chip.hpp"
#include "iso7816profile.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>


namespace logicalaccess
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
