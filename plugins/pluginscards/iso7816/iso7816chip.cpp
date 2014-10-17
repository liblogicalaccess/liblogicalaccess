/**
 * \file iso7816chip.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief ISO7816 chip.
 */

#include "iso7816chip.hpp"
#include "iso7816profile.hpp"
#include "iso7816storagecardservice.hpp"
#include "iso7816nfctag4cardservice.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>

namespace logicalaccess
{
    ISO7816Chip::ISO7816Chip(std::string ct)
        : Chip(ct)
    {
        d_profile.reset(new ISO7816Profile());
    }

    ISO7816Chip::ISO7816Chip()
        : Chip(CHIP_ISO7816)
    {
        d_profile.reset(new ISO7816Profile());
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
        case CST_ACCESS_CONTROL:
            break;
        case CST_NFC_TAG:
            service.reset(new ISO7816NFCTag4CardService(shared_from_this()));
            break;
        }

        return service;
    }
}