/**
 * \file mifareultralightcchip.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Mifare Ultralight C chip.
 */

#include "mifareultralightcchip.hpp"
#include "mifareultralightcstoragecardservice.hpp"

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

	boost::shared_ptr<CardService> MifareUltralightCChip::getService(CardServiceType serviceType)
	{
		boost::shared_ptr<CardService> service;

		switch (serviceType)
		{
		case CST_STORAGE:
			service.reset(new MifareUltralightCStorageCardService(shared_from_this()));
			break;
		case CST_ACCESS_CONTROL:
		  break;
		case CST_NFC_TAG:
		  break;
		}

		return service;
	}
}
