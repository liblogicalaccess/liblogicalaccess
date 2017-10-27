/**
 * \file generictagchip.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Generic tag chip.
 */

#include "generictagchip.hpp"
#include "generictagaccesscontrolcardservice.hpp"
#include "logicalaccess/cards/locationnode.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>

namespace logicalaccess
{
GenericTagChip::GenericTagChip(std::string cardtype)
    : Chip(cardtype)
{
    d_tagIdBitsLength = 0;
}

GenericTagChip::GenericTagChip()
    : Chip(CHIP_GENERICTAG)
{
    d_tagIdBitsLength = 0;
}

GenericTagChip::~GenericTagChip()
{
}

std::shared_ptr<LocationNode> GenericTagChip::getRootLocationNode()
{
    if (d_real_chip)
    {
        return d_real_chip->getRootLocationNode();
    }
    std::shared_ptr<LocationNode> rootNode;
    rootNode.reset(new LocationNode());

    rootNode->setName("RFID Tag");

    return rootNode;
}

void GenericTagChip::setRealChip(std::shared_ptr<Chip> real_chip)
{
    d_real_chip = real_chip;
}

std::shared_ptr<CardService> GenericTagChip::getService(CardServiceType serviceType)
{
    std::shared_ptr<CardService> service;

    if (d_real_chip)
    {
        service = d_real_chip->getService(serviceType);
    }
    else
    {
        switch (serviceType)
        {
        case CST_ACCESS_CONTROL:
        {
            service.reset(new GenericTagAccessControlCardService(shared_from_this()));
        }
        break;
        default: break;
        }
    }

    if (!service)
    {
        service = Chip::getService(serviceType);
    }

    return service;
}
}