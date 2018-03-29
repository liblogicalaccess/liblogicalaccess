/**
 * \file seoschip.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief SEOS chip.
 */

#include <logicalaccess/plugins/cards/seos/seoschip.hpp>
#include <logicalaccess/services/accesscontrol/accesscontrolcardservice.hpp>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <logicalaccess/dynlibrary/librarymanager.hpp>
#include <cassert>
#include <logicalaccess/cards/locationnode.hpp>

namespace logicalaccess
{
SEOSChip::SEOSChip()
    : Chip(CHIP_SEOS)
{
}

SEOSChip::~SEOSChip()
{
}

std::shared_ptr<LocationNode> SEOSChip::getRootLocationNode()
{
    std::shared_ptr<LocationNode> rootNode;
    rootNode.reset(new LocationNode());

    rootNode->setName("HID SEOS");

    return rootNode;
}

std::shared_ptr<CardService> SEOSChip::getService(CardServiceType serviceType)
{
    std::shared_ptr<CardService> service;

    switch (serviceType)
    {
    case CST_ACCESS_CONTROL:
    {
        service = LibraryManager::getInstance()->getCardService(shared_from_this(),
                                                                CST_ACCESS_CONTROL);
        if (!service)
            service.reset(new AccessControlCardService(shared_from_this()));
    }
    break;
    default: break;
    }

    if (!service)
    {
        service = Chip::getService(serviceType);
    }

    return service;
}
}
