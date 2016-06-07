/**
 * \file topazchip.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Topaz chip.
 */

#include "topazchip.hpp"
#include "logicalaccess/services/accesscontrol/accesscontrolcardservice.hpp"
#include "topazstoragecardservice.hpp"
#include "logicalaccess/cards/locationnode.hpp"
#include "nfctag1cardservice.hpp"

namespace logicalaccess
{
    TopazChip::TopazChip()
        : Chip(CHIP_TOPAZ)
    {
        d_profile.reset(new TopazProfile());
        d_nbblocks = 15;
    }

    TopazChip::~TopazChip()
    {
    }

    unsigned short TopazChip::getNbBlocks(bool checkOnCard)
	{
		if (checkOnCard)
		{
			try
			{
                std::shared_ptr<TopazCommands> tzcmd = getTopazCommands();
                d_nbblocks = 15; // Topaz120
                tzcmd->readPage(63);
				d_nbblocks = 64; // Topaz512
			}
			catch (std::exception&) { }
		}

		return d_nbblocks;
	}

    void TopazChip::checkRootLocationNodeName(std::shared_ptr<LocationNode> rootNode)
    {
        unsigned short nbblocks = getNbBlocks(true);

        switch (nbblocks)
        {
        case 64:
            rootNode->setName("Topaz512");
            break;
        }
    }

    std::shared_ptr<LocationNode> TopazChip::getRootLocationNode()
    {
        std::shared_ptr<LocationNode> rootNode;
        rootNode.reset(new LocationNode());

        rootNode->setName("Topaz");
        checkRootLocationNodeName(rootNode);

        for (unsigned short i = 0; i < getNbBlocks(); ++i)
        {
            addPageNode(rootNode, i);
        }

        return rootNode;
    }

    void TopazChip::addPageNode(std::shared_ptr<LocationNode> rootNode, int page)
    {
        char tmpName[255];
        std::shared_ptr<LocationNode> sectorNode;
        sectorNode.reset(new LocationNode());

        sprintf(tmpName, "Page %d", page);
        sectorNode->setName(tmpName);
        sectorNode->setLength(8);
        sectorNode->setNeedAuthentication(true);

        std::shared_ptr<TopazLocation> location;
        location.reset(new TopazLocation());
        location->page = page;
        location->byte = 0;

        sectorNode->setLocation(location);
        sectorNode->setParent(rootNode);
        rootNode->getChildrens().push_back(sectorNode);
    }

    std::shared_ptr<CardService> TopazChip::getService(CardServiceType serviceType)
    {
        std::shared_ptr<CardService> service;

        switch (serviceType)
        {
        case CST_ACCESS_CONTROL:
            service.reset(new AccessControlCardService(shared_from_this()));
            break;
        case CST_STORAGE:
            service.reset(new TopazStorageCardService(shared_from_this()));
            break;
		case CST_NFC_TAG:
			service.reset(new NFCTag1CardService(shared_from_this()));
        default:
            break;
        }

        return service;
    }
}