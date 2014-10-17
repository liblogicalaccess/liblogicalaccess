/**
 * \file mifarechip.cpp
 * \author Xavier SCHMERBER <xavier.schmerber@gmail.com>
 * \brief Mifare chip.
 */

#include "mifarepluschip.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>

#include "logicalaccess/services/accesscontrol/accesscontrolcardservice.hpp"
#include "logicalaccess/services/accesscontrol/formats/bithelper.hpp"
#include "mifareplussl1storagecardservice.hpp"
#include "mifareplussl3storagecardservice.hpp"

#define MIFARE_PLUS_2K_SECTOR_NB 32

namespace logicalaccess
{
    MifarePlusChip::MifarePlusChip()
        : Chip(CHIP_MIFAREPLUS4K), d_nbSectors(MIFARE_PLUS_2K_SECTOR_NB)
    {
        d_profile.reset();
    }

    MifarePlusChip::MifarePlusChip(std::string cardtype, unsigned int nbSectors) :
        Chip(cardtype), d_nbSectors(nbSectors)
    {
        d_profile.reset();
    }

    MifarePlusChip::~MifarePlusChip()
    {
    }

    unsigned int MifarePlusChip::getNbSectors() const
    {
        return d_nbSectors;
    }

    void MifarePlusChip::addSectorNode(std::shared_ptr<LocationNode> rootNode, int sector)
    {
        char tmpName[255];
        std::shared_ptr<LocationNode> sectorNode;
        sectorNode.reset(new LocationNode());

        sprintf(tmpName, "Sector %d", sector);
        sectorNode->setName(tmpName);
        sectorNode->setLength((sector >= MIFARE_PLUS_2K_SECTOR_NB) ? 256 : 64);
        sectorNode->setNeedAuthentication(true);

        std::shared_ptr<MifarePlusLocation> location;
        location.reset(new MifarePlusLocation());
        location->sector = sector;
        location->byte = 0;
        location->block = sector * ((sector >= MIFARE_PLUS_2K_SECTOR_NB) ? ((4 * MIFARE_PLUS_2K_SECTOR_NB) + (16 * (sector - MIFARE_PLUS_2K_SECTOR_NB))) : 4);

        sectorNode->setLocation(location);
        sectorNode->setParent(rootNode);
        rootNode->getChildrens().push_back(sectorNode);
    }

    std::shared_ptr<LocationNode> MifarePlusChip::getRootLocationNode()
    {
        std::shared_ptr<LocationNode> rootNode;
        rootNode.reset(new LocationNode());

        rootNode->setName("Mifare Plus");
        std::shared_ptr<MifarePlusLocation> rootLocation;
        rootLocation.reset(new MifarePlusLocation());
        rootLocation->sector = (unsigned int)-1;
        rootNode->setLocation(rootLocation);

        if (getCommands())
        {
            for (int i = 0; i < MIFARE_PLUS_2K_SECTOR_NB; i++)
            {
                addSectorNode(rootNode, i);
            }
        }

        return rootNode;
    }

    std::shared_ptr<CardService> MifarePlusChip::getService(CardServiceType serviceType)
    {
        std::shared_ptr<CardService> service;

        switch (serviceType)
        {
        case CST_ACCESS_CONTROL:
        {
            service.reset(new AccessControlCardService(shared_from_this()));
        }
            break;
        case CST_STORAGE:
        {
            if (getMifarePlusSL1Commands())
            {
                service.reset(new MifarePlusSL1StorageCardService(shared_from_this()));
            }
            else if (getMifarePlusSL3Commands())
            {
                service.reset(new MifarePlusSL3StorageCardService(shared_from_this()));
            }
        }
            break;
        case CST_NFC_TAG:
            break;
        }

        if (!service)
        {
            service = Chip::getService(serviceType);
        }

        return service;
    }
}