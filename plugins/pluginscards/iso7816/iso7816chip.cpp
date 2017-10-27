/**
 * \file iso7816chip.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief ISO7816 chip.
 */

#include "iso7816chip.hpp"
#include "iso7816storagecardservice.hpp"
#include "iso7816nfctag4cardservice.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>
#include "logicalaccess/cards/locationnode.hpp"

namespace logicalaccess
{
ISO7816Chip::ISO7816Chip(std::string ct)
    : Chip(ct)
{
}

ISO7816Chip::ISO7816Chip()
    : Chip(CHIP_ISO7816)
{
}

ISO7816Chip::~ISO7816Chip()
{
}

std::shared_ptr<LocationNode> ISO7816Chip::getRootLocationNode()
{
    std::shared_ptr<LocationNode> rootNode;
    rootNode.reset(new LocationNode());
    rootNode->setName("ISO 7816");

    return rootNode;
}

std::shared_ptr<CardService> ISO7816Chip::getService(CardServiceType serviceType)
{
    std::shared_ptr<CardService> service;

    switch (serviceType)
    {
    case CST_STORAGE:
        service.reset(new ISO7816StorageCardService(shared_from_this()));
        break;
    case CST_NFC_TAG:
        service.reset(new ISO7816NFCTag4CardService(shared_from_this()));
        break;
    default: break;
    }

    return service;
}

std::shared_ptr<Location> ISO7816Chip::createLocation() const
{
    std::shared_ptr<ISO7816Location> ret;
    ret.reset(new ISO7816Location());
    return ret;
}
}