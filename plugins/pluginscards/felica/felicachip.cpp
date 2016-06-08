/**
 * \file felicachip.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief FeliCa chip.
 */

#include "felicachip.hpp"
#include "felicaprofile.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>

#include "logicalaccess/services/accesscontrol/formats/bithelper.hpp"
#include "felicastoragecardservice.hpp"
#include "nfctag3cardservice.hpp"
#include "logicalaccess/cards/locationnode.hpp"

namespace logicalaccess
{
	FeliCaChip::FeliCaChip(std::string ct)
        : Chip(ct)
    {
        d_profile.reset(new FeliCaProfile());
    }

	FeliCaChip::FeliCaChip()
        : Chip(CHIP_FELICA)
    {
		d_profile.reset(new FeliCaProfile());
    }

	FeliCaChip::~FeliCaChip()
    {
    }

	std::shared_ptr<LocationNode> FeliCaChip::getRootLocationNode()
    {
        std::shared_ptr<LocationNode> rootNode;
        rootNode.reset(new LocationNode());

        rootNode->setName("FeliCa");
        rootNode->setHasProperties(true);

        std::shared_ptr<FeliCaLocation> rootLocation;
		rootLocation.reset(new FeliCaLocation());
        //FIXME: We need to add a specific property for this
        rootLocation->code = -1;
        rootLocation->block = static_cast<int>(-1);
        rootNode->setLocation(rootLocation);

        for (unsigned short i = 0; i < 14; ++i)
        {
            addBlockNode(rootNode, FELICA_CODE_NDEF_READ, i);
        }

        return rootNode;
    }

    void FeliCaChip::addBlockNode(std::shared_ptr<LocationNode> rootNode, unsigned short code, unsigned short block)
    {
        char tmpName[255];
        std::shared_ptr<LocationNode> blockNode;
        blockNode.reset(new LocationNode());

        sprintf(tmpName, "Block %d", block);
        blockNode->setName(tmpName);
        blockNode->setLength(16);
        blockNode->setNeedAuthentication(true);

        std::shared_ptr<FeliCaLocation> location;
        location.reset(new FeliCaLocation());
        location->code = code;
        location->block = block;

        blockNode->setLocation(location);
        blockNode->setParent(rootNode);
        rootNode->getChildrens().push_back(blockNode);
    }

    std::shared_ptr<CardService> FeliCaChip::getService(CardServiceType serviceType)
    {
        std::shared_ptr<CardService> service;

        switch (serviceType)
        {
        case CST_STORAGE:
            service.reset(new FeliCaStorageCardService(shared_from_this()));
            break;
        case CST_NFC_TAG:
            service.reset(new NFCTag3CardService(shared_from_this()));
            break;
        default:
            break;
        }

        return service;
    }
}