/**
 * \file proxchip.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Prox chip.
 */

#include "proxchip.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>
#include <logicalaccess/dynlibrary/librarymanager.hpp>

#include "proxaccesscontrolcardservice.hpp"

namespace logicalaccess
{
    ProxChip::ProxChip()
        : Chip("Prox")
    {
        d_profile.reset(new ProxProfile());
    }

    ProxChip::~ProxChip()
    {
    }

    std::shared_ptr<LocationNode> ProxChip::getRootLocationNode()
    {
        std::shared_ptr<LocationNode> rootNode;
        rootNode.reset(new LocationNode());

        rootNode->setName("HID Prox");

        return rootNode;
    }

    std::shared_ptr<CardService> ProxChip::getService(CardServiceType serviceType)
    {
        std::shared_ptr<CardService> service;

        switch (serviceType)
        {
        case CST_ACCESS_CONTROL:
        {
            service = LibraryManager::getInstance()
                    ->getAccessControlCardService(shared_from_this());
            if (!service)
                service.reset(new ProxAccessControlCardService(shared_from_this()));
        }
            break;
        case CST_NFC_TAG:
            break;
        case CST_STORAGE:
            break;
        }

        if (!service)
        {
            service = Chip::getService(serviceType);
        }

        return service;
    }
}