/**
 * \file MifareUltralightChip.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Mifare Ultralight chip.
 */

#include "MifareUltralightChip.h"

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
	MifareUltralightChip::MifareUltralightChip(std::string ct) : 
		Chip(ct)
	{
		
	}

	MifareUltralightChip::MifareUltralightChip()
		: Chip(CHIP_MIFAREULTRALIGHT)
	{
		d_profile.reset(new MifareUltralightProfile());
	}

	MifareUltralightChip::~MifareUltralightChip()
	{
	}

	boost::shared_ptr<LocationNode> MifareUltralightChip::getRootLocationNode()
	{
		boost::shared_ptr<LocationNode> rootNode;
		rootNode.reset(new LocationNode());

		rootNode->setName("Mifare Ultralight");

		for (unsigned int i = 0; i < 16; ++i)
		{
			addPageNode(rootNode, i);
		}

		return rootNode;
	}

	void MifareUltralightChip::addPageNode(boost::shared_ptr<LocationNode> rootNode, int page)
	{
		char tmpName[255];
		boost::shared_ptr<LocationNode> sectorNode;
		sectorNode.reset(new LocationNode());

		sprintf(tmpName, "Page %u", page);
		sectorNode->setName(tmpName);
		sectorNode->setLength(4);
		sectorNode->setNeedAuthentication(true);

		boost::shared_ptr<MifareUltralightLocation> location;
		location.reset(new MifareUltralightLocation());
		location->page = page;
		location->byte = 0;

		sectorNode->setLocation(location);
		sectorNode->setParent(rootNode);
		rootNode->getChildrens().push_back(sectorNode);
	}
}
