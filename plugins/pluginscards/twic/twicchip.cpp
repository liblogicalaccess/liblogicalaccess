/**
 * \file twicchip.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Twic chip.
 */

#include "twicchip.hpp"
#include "twicprofile.hpp"
#include "twiclocation.hpp"
#include "twicaccesscontrolcardservice.hpp"
#include "twicstoragecardservice.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>

namespace logicalaccess
{
    TwicChip::TwicChip()
        : ISO7816Chip(CHIP_TWIC)
    {
        d_profile.reset(new TwicProfile());
    }

    TwicChip::~TwicChip()
    {
    }

    boost::shared_ptr<LocationNode> TwicChip::getRootLocationNode()
    {
        boost::shared_ptr<LocationNode> rootNode;
        rootNode.reset(new LocationNode());
        rootNode->setName("Twic");

        boost::shared_ptr<LocationNode> applNode;
        applNode.reset(new LocationNode());
        applNode->setName("Twic Application");
        applNode->setParent(rootNode);
        rootNode->getChildrens().push_back(applNode);

        boost::shared_ptr<LocationNode> ucuiNode;
        ucuiNode.reset(new LocationNode());
        ucuiNode->setName("Unsigned Cardholder Unique Identifier");
        boost::shared_ptr<TwicLocation> ucuiLocation;
        ucuiLocation.reset(new TwicLocation());
        ucuiLocation->dataObject = 0x5FC104;
        ucuiNode->setLocation(ucuiLocation);
        ucuiNode->setNeedAuthentication(true);
        ucuiNode->setLength(getTwicCommands()->getDataObjectLength(ucuiLocation->dataObject, true));
        ucuiNode->setParent(applNode);
        applNode->getChildrens().push_back(ucuiNode);

        boost::shared_ptr<LocationNode> tpkNode;
        tpkNode.reset(new LocationNode());
        tpkNode->setName("TWIC Privacy Key");
        boost::shared_ptr<TwicLocation> tpkLocation;
        tpkLocation.reset(new TwicLocation());
        tpkLocation->dataObject = 0xDFC101;
        tpkNode->setLocation(tpkLocation);
        tpkNode->setNeedAuthentication(true);
        tpkNode->setLength(getTwicCommands()->getDataObjectLength(tpkLocation->dataObject, true));
        tpkNode->setParent(applNode);
        applNode->getChildrens().push_back(tpkNode);

        boost::shared_ptr<LocationNode> cuiNode;
        cuiNode.reset(new LocationNode());
        cuiNode->setName("Cardholder Unique Identifier");
        boost::shared_ptr<TwicLocation> cuiLocation;
        cuiLocation.reset(new TwicLocation());
        cuiLocation->dataObject = 0x5FC102;
        cuiNode->setLocation(cuiLocation);
        cuiNode->setNeedAuthentication(true);
        cuiNode->setLength(getTwicCommands()->getDataObjectLength(cuiLocation->dataObject, true));
        cuiNode->setParent(applNode);
        applNode->getChildrens().push_back(cuiNode);

        boost::shared_ptr<LocationNode> cfNode;
        cfNode.reset(new LocationNode());
        cfNode->setName("Cardholder Fingerprints");
        boost::shared_ptr<TwicLocation> cfLocation;
        cfLocation.reset(new TwicLocation());
        cfLocation->dataObject = 0xDFC103;
        cfNode->setLocation(cfLocation);
        cfNode->setNeedAuthentication(true);
        cfNode->setLength(getTwicCommands()->getDataObjectLength(cfLocation->dataObject, true));
        cfNode->setParent(applNode);
        applNode->getChildrens().push_back(cfNode);

        boost::shared_ptr<LocationNode> soNode;
        soNode.reset(new LocationNode());
        soNode->setName("Security Object");
        boost::shared_ptr<TwicLocation> soLocation;
        soLocation.reset(new TwicLocation());
        soLocation->dataObject = 0xDFC10F;
        soNode->setLocation(soLocation);
        soNode->setNeedAuthentication(true);
        soNode->setLength(getTwicCommands()->getDataObjectLength(soLocation->dataObject, true));
        soNode->setParent(applNode);
        applNode->getChildrens().push_back(soNode);

        return rootNode;
    }

    boost::shared_ptr<CardService> TwicChip::getService(CardServiceType serviceType)
    {
        boost::shared_ptr<CardService> service;

        switch (serviceType)
        {
        case CST_ACCESS_CONTROL:
        {
            service.reset(new TwicAccessControlCardService(shared_from_this()));
        }
            break;
        case CST_STORAGE:
        {
            service.reset(new TwicStorageCardService(shared_from_this()));
        }
            break;
        case CST_NFC_TAG:
            break;
        }

        if (!service)
        {
            service = ISO7816Chip::getService(serviceType);
        }

        return service;
    }
}