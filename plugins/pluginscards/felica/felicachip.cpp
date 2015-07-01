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
        rootLocation->block = static_cast<int>(-1);
        rootNode->setLocation(rootLocation);

        return rootNode;
    }

    std::shared_ptr<CardService> FeliCaChip::getService(CardServiceType serviceType)
    {
        std::shared_ptr<CardService> service;

        switch (serviceType)
        {
        case CST_STORAGE:
            service.reset(new FeliCaStorageCardService(shared_from_this()));
            break;
        case CST_ACCESS_CONTROL:
            break;
        case CST_NFC_TAG:
            break;
        }

        return service;
    }
}