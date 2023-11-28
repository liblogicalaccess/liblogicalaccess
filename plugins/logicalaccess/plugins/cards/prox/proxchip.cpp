/**
 * \file proxchip.cpp
 * \author Maxime C. <maxime@leosac.com>
 * \brief Prox chip.
 */

#include <logicalaccess/plugins/cards/prox/proxchip.hpp>
#include <logicalaccess/plugins/cards/prox/proxlocation.hpp>

#include <iostream>
#include <iomanip>
#include <sstream>
#include <logicalaccess/dynlibrary/librarymanager.hpp>
#include <cassert>

#include <logicalaccess/plugins/cards/prox/proxaccesscontrolcardservice.hpp>
#include <logicalaccess/cards/locationnode.hpp>

namespace logicalaccess
{
ProxChip::ProxChip()
    : Chip("Prox")
{
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
        service = LibraryManager::getInstance()->getCardService(shared_from_this(),
                                                                CST_ACCESS_CONTROL);
        if (!service)
            service.reset(new ProxAccessControlCardService(shared_from_this()));
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

std::shared_ptr<Location> ProxChip::createLocation() const
{
    std::shared_ptr<ProxLocation> ret;
    ret.reset(new ProxLocation());
    return ret;
}
}