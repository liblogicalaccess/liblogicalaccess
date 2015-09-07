/**
 * \file mifareultralightcchip.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Mifare Ultralight C chip.
 */

#include "mifareultralightcchip.hpp"
#include "mifareultralightcstoragecardservice.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>
#include "logicalaccess/services/accesscontrol/accesscontrolcardservice.hpp"
#include "logicalaccess/cards/locationnode.hpp"

namespace logicalaccess
{
    MifareUltralightCChip::MifareUltralightCChip()
        : MifareUltralightChip(CHIP_MIFAREULTRALIGHTC)
    {
        d_profile.reset(new MifareUltralightCProfile());
    }

    MifareUltralightCChip::~MifareUltralightCChip()
    {
    }

    std::shared_ptr<LocationNode> MifareUltralightCChip::getRootLocationNode()
    {
        std::shared_ptr<LocationNode> rootNode;
        rootNode.reset(new LocationNode());

        rootNode->setName("Mifare Ultralight C");

        for (unsigned int i = 0; i < 48; ++i)
        {
            addPageNode(rootNode, i);
        }

        return rootNode;
    }

    std::shared_ptr<CardService> MifareUltralightCChip::getService(CardServiceType serviceType)
    {
        std::shared_ptr<CardService> service;

        switch (serviceType)
        {
        case CST_ACCESS_CONTROL:
            service.reset(new AccessControlCardService(shared_from_this()));
            break;
        case CST_STORAGE:
            service.reset(new MifareUltralightCStorageCardService(shared_from_this()));
            break;
        case CST_NFC_TAG:
            break;
        }

        return service;
    }
}