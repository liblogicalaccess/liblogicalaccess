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
        std::shared_ptr<LocationNode> rootNode;
        rootNode.reset(new LocationNode());

        rootNode->setName("RFID Tag");

        return rootNode;
    }

    std::shared_ptr<CardService> GenericTagChip::getService(CardServiceType serviceType)
    {
        std::shared_ptr<CardService> service;

        switch (serviceType)
        {
        case CST_ACCESS_CONTROL:
        {
            service.reset(new GenericTagAccessControlCardService(shared_from_this()));
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