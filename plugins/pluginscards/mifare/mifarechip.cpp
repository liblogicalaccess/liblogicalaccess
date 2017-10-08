/**
 * \file mifarechip.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Mifare chip.
 */

#include "mifarechip.hpp"

#include <iomanip>
#include <logicalaccess/dynlibrary/librarymanager.hpp>
#include "logicalaccess/services/accesscontrol/accesscontrolcardservice.hpp"
#include "mifarestoragecardservice.hpp"
#include "mifarenfctagcardservice.hpp"
#include "logicalaccess/cards/locationnode.hpp"

namespace logicalaccess
{
    MifareChip::MifareChip()
        : Chip("Mifare"), d_nbSectors(16)
    {
    }

	MifareChip::MifareChip(std::string cardtype)
		: Chip(cardtype), d_nbSectors(16)
	{
	}

    MifareChip::MifareChip(std::string cardtype, unsigned int nbSectors) :
        Chip(cardtype), d_nbSectors(nbSectors)
    {
    }

    MifareChip::~MifareChip()
    {
    }

    unsigned int MifareChip::getNbSectors() const
    {
        return d_nbSectors;
    }

    void MifareChip::addSectorNode(std::shared_ptr<LocationNode> rootNode, int sector)
    {
        char tmpName[255];
        std::shared_ptr<LocationNode> sectorNode;
        sectorNode.reset(new LocationNode());

        sprintf(tmpName, "Sector %d", sector);
        sectorNode->setName(tmpName);
        sectorNode->setLength((sector >= 32) ? 240 : 48);
        sectorNode->setNeedAuthentication(true);

        for (unsigned char i = 0; i < (sectorNode->getLength() / 16); ++i)
		{
			addBlockNode(sectorNode, sector, i);
		}

        std::shared_ptr<MifareLocation> location;
        location.reset(new MifareLocation());
        location->sector = sector;
		location->block = -1;
        location->byte = 0;

        sectorNode->setLocation(location);
        sectorNode->setParent(rootNode);
        rootNode->getChildrens().push_back(sectorNode);
    }

	void MifareChip::addBlockNode(std::shared_ptr<LocationNode> rootNode, int sector, unsigned char block)
	{
		char tmpName[255];
		std::shared_ptr<LocationNode> blockNode;
		blockNode.reset(new LocationNode());

		sprintf(tmpName, "Block %d", block);
		blockNode->setName(tmpName);
		blockNode->setLength(16);
		blockNode->setNeedAuthentication(true);

		std::shared_ptr<MifareLocation> location;
		location.reset(new MifareLocation());
		location->sector = sector;
		location->block = block;
		location->byte = 0;

		blockNode->setLocation(location);
		blockNode->setParent(rootNode);
		rootNode->getChildrens().push_back(blockNode);
	}

    std::shared_ptr<LocationNode> MifareChip::getRootLocationNode()
    {
        std::shared_ptr<LocationNode> rootNode;
        rootNode.reset(new LocationNode());

        rootNode->setName("Mifare Classic");
        std::shared_ptr<MifareLocation> rootLocation;
        rootLocation.reset(new MifareLocation());
        rootLocation->sector = (unsigned int)-1;
        rootNode->setLocation(rootLocation);

        if (getCommands())
        {
            for (int i = 0; i < (int)getNbSectors(); i++)
            {
                addSectorNode(rootNode, i);
            }
        }

        return rootNode;
    }

    std::shared_ptr<CardService> MifareChip::getService(CardServiceType serviceType)
    {
        std::shared_ptr<CardService> service;

        switch (serviceType)
        {
        case CST_ACCESS_CONTROL:
        {
            service = LibraryManager::getInstance()
                    ->getCardService(shared_from_this(), CardServiceType::CST_ACCESS_CONTROL);
            if (!service)
                service.reset(new AccessControlCardService(shared_from_this()));
        }
            break;
        case CST_STORAGE:
        {
            service.reset(new MifareStorageCardService(shared_from_this()));
        }
            break;
        case CST_NFC_TAG:
        {
            service.reset(new MifareNFCTagCardService(shared_from_this()));
        }
            break;
        case CST_UID_CHANGER:
        {
            // Changing the UID of Mifare Classic works with (some) NFC only.
            // Therefore, we relies on the LibraryManager to (maybe) retrieve
            // a UIDChanger service.
            service = LibraryManager::getInstance()->getCardService(shared_from_this(),
                                                                    CST_UID_CHANGER);
            // If the LLA-NFC plugin doesn't give us a service, for any reason
            // (not a NFC reader for example) then we do nothing.
            break;
        }
        default:
            break;
        }

        if (!service)
        {
            service = Chip::getService(serviceType);
        }

        return service;
    }

	std::shared_ptr<AccessInfo> MifareChip::createAccessInfo() const
	{
		std::shared_ptr<MifareAccessInfo> ret;
		ret.reset(new MifareAccessInfo());
		return ret;
	}

	std::shared_ptr<Location> MifareChip::createLocation() const
	{
		std::shared_ptr<MifareLocation> ret;
		ret.reset(new MifareLocation());
		return ret;
	}
}