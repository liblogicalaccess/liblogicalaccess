/**
 * \file cps3chip.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief CPS3 chip.
 */

#include "cps3chip.hpp"
#include "cps3profile.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>

#include "logicalaccess/services/accesscontrol/formats/bithelper.hpp"
#include "cps3storagecardservice.hpp"
#include "logicalaccess/cards/locationnode.hpp"

namespace logicalaccess
{
	CPS3Chip::CPS3Chip(std::string ct)
        : ISO7816Chip(ct)
    {
        d_profile.reset(new CPS3Profile());
    }

	CPS3Chip::CPS3Chip()
        : ISO7816Chip(CHIP_CPS3)
    {
		d_profile.reset(new CPS3Profile());
    }

	CPS3Chip::~CPS3Chip()
    {
    }

	std::shared_ptr<LocationNode> CPS3Chip::getRootLocationNode()
    {
        std::shared_ptr<LocationNode> rootNode;
        rootNode.reset(new LocationNode());

        rootNode->setName("CPS3");
        rootNode->setHasProperties(true);

        std::shared_ptr<CPS3Location> rootLocation;
		rootLocation.reset(new CPS3Location());
        rootNode->setLocation(rootLocation);

        return rootNode;
    }

    std::shared_ptr<CardService> CPS3Chip::getService(CardServiceType serviceType)
    {
        std::shared_ptr<CardService> service;

        switch (serviceType)
        {
        case CST_STORAGE:
            service.reset(new CPS3StorageCardService(shared_from_this()));
            break;
        case CST_ACCESS_CONTROL:
            break;
        case CST_NFC_TAG:
            break;
        }

        return service;
    }
}