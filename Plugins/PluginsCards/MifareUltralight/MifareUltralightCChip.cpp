/**
 * \file MifareUltralightCChip.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Mifare Ultralight C chip.
 */

#include "MifareUltralightCChip.h"

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
	MifareUltralightCChip::MifareUltralightCChip()
		: MifareUltralightChip(CHIP_MIFAREULTRALIGHTC)
	{
		d_profile.reset(new MifareUltralightCProfile());
	}

	MifareUltralightCChip::~MifareUltralightCChip()
	{
	}

	boost::shared_ptr<LocationNode> MifareUltralightCChip::getRootLocationNode()
	{
		boost::shared_ptr<LocationNode> rootNode;
		rootNode.reset(new LocationNode());

		rootNode->setName("Mifare Ultralight C");

		for (unsigned int i = 0; i < 48; ++i)
		{
			addPageNode(rootNode, i);
		}

		return rootNode;
	}
}