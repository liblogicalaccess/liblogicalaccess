/**
 * \file mifareultralightchip.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Mifare Ultralight chip.
 */

#include "mifareultralightchip.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>

#include "logicalaccess/services/accesscontrol/formats/bithelper.hpp"
#include "mifareultralightstoragecardservice.hpp"

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

		sprintf(tmpName, "Page %d", page);
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

	boost::shared_ptr<CardService> MifareUltralightChip::getService(CardServiceType serviceType)
	{
		boost::shared_ptr<CardService> service;

		switch (serviceType)
		{
		case CST_STORAGE:
			service.reset(new MifareUltralightStorageCardService(shared_from_this()));
			break;
		case CST_NFC_TAG:
		  break;
		case CST_ACCESS_CONTROL:
		  break;
		}

		return service;
	}
}
