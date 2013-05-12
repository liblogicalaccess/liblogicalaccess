/**
 * \file mifareultralightcchip.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Mifare Ultralight C chip.
 */

#include "mifareultralightcchip.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>

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