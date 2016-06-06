/**
 * \file mifareultralightchip.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Mifare Ultralight chip.
 */

#include "mifareultralightchip.hpp"
#include "logicalaccess/services/accesscontrol/accesscontrolcardservice.hpp"
#include "mifareultralightstoragecardservice.hpp"
#include "logicalaccess/cards/locationnode.hpp"
#include "nfctag2cardservice.hpp"
#include "mifareultralightuidchangerservice.hpp"

namespace logicalaccess
{
    MifareUltralightChip::MifareUltralightChip(std::string ct) :
		Chip(ct), d_nbblocks(16)
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

	unsigned short MifareUltralightChip::getNbBlocks(bool checkOnCard)
	{
		if (checkOnCard)
		{
			try
			{
				std::shared_ptr<MifareUltralightCommands> mfucmd = getMifareUltralightCommands();
				d_nbblocks = 16;
				mfucmd->readPage(31);
				d_nbblocks = 32;
				mfucmd->readPage(35);
				d_nbblocks = 36;
				mfucmd->readPage(41);
				d_nbblocks = 42;
				mfucmd->readPage(125);
				d_nbblocks = 126;
				mfucmd->readPage(221);
				d_nbblocks = 222;
			}
			catch (std::exception& ex) { }
		}

		return d_nbblocks;
	}

    std::shared_ptr<LocationNode> MifareUltralightChip::getRootLocationNode()
    {
        std::shared_ptr<LocationNode> rootNode;
        rootNode.reset(new LocationNode());

		switch (d_nbblocks)
		{
		case 32:
			rootNode->setName("NTAG212");
			break;
		case 36:
			rootNode->setName("NTAG213");
			break;
		case 42:
			rootNode->setName("NTAG203F");
			break;
		case 126:
			rootNode->setName("NTAG215");
			break;
		case 222:
			rootNode->setName("NTAG216");
			break;
		default:
			rootNode->setName("Mifare Ultralight");
			break;
		}

		for (unsigned short i = 0; i < getNbBlocks(true); ++i)
        {
            addPageNode(rootNode, i);
        }

        return rootNode;
    }

    void MifareUltralightChip::addPageNode(std::shared_ptr<LocationNode> rootNode, int page)
    {
        char tmpName[255];
        std::shared_ptr<LocationNode> sectorNode;
        sectorNode.reset(new LocationNode());

        sprintf(tmpName, "Page %d", page);
        sectorNode->setName(tmpName);
        sectorNode->setLength(4);
        sectorNode->setNeedAuthentication(true);

        std::shared_ptr<MifareUltralightLocation> location;
        location.reset(new MifareUltralightLocation());
        location->page = page;
        location->byte = 0;

        sectorNode->setLocation(location);
        sectorNode->setParent(rootNode);
        rootNode->getChildrens().push_back(sectorNode);
    }

    std::shared_ptr<CardService> MifareUltralightChip::getService(CardServiceType serviceType)
    {
        std::shared_ptr<CardService> service;

        switch (serviceType)
        {
        case CST_ACCESS_CONTROL:
            service.reset(new AccessControlCardService(shared_from_this()));
            break;
        case CST_STORAGE:
            service.reset(new MifareUltralightStorageCardService(shared_from_this()));
            break;
		case CST_NFC_TAG:
			service.reset(new NFCTag2CardService(shared_from_this()));
			break;
        case CST_UID_CHANGER:
        {
            auto storage = std::make_shared<MifareUltralightStorageCardService>(
                shared_from_this());
            service.reset(new MifareUltralightUIDChangerService(storage));
            break;
        }
        default:
            break;
        }

        return service;
    }
}