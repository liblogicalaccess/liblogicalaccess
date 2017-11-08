/**
 * \file desfireev1chip.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief DESFire EV1 chip.
 */

#include <logicalaccess/plugins/cards/desfire/desfireev1chip.hpp>
#include <logicalaccess/plugins/cards/desfire/desfireev1location.hpp>
#include <logicalaccess/services/accesscontrol/accesscontrolcardservice.hpp>
#include <logicalaccess/plugins/cards/desfire/desfirestoragecardservice.hpp>
#include <logicalaccess/plugins/cards/desfire/desfireev1nfctag4cardservice.hpp>

#include <cstring>
#include <logicalaccess/dynlibrary/librarymanager.hpp>
#include <logicalaccess/cards/locationnode.hpp>

namespace logicalaccess
{
DESFireEV1Chip::DESFireEV1Chip()
    : DESFireChip(CHIP_DESFIRE_EV1)
{
}

DESFireEV1Chip::DESFireEV1Chip(std::string ct)
    : DESFireChip(ct)
{
}

DESFireEV1Chip::~DESFireEV1Chip()
{
}

std::shared_ptr<LocationNode> DESFireEV1Chip::getRootLocationNode()
{
    std::shared_ptr<LocationNode> rootNode = DESFireChip::getRootLocationNode();
    rootNode->setName("Mifare DESFire EV1");

    return rootNode;
}

std::shared_ptr<DESFireLocation> DESFireEV1Chip::getApplicationLocation()
{
    std::shared_ptr<DESFireEV1Location> location(new DESFireEV1Location());

    try
    {
        DESFireKeySettings keySettings;
        unsigned char maxNbkeys = 0;
        DESFireKeyType keyType;
        getDESFireEV1Commands()->getKeySettings(keySettings, maxNbkeys, keyType);

        if (keyType != DF_KEY_DES)
        {
            location->useEV1       = true;
            location->cryptoMethod = keyType;
        }
    }
    catch (std::exception &)
    {
    }

    return location;
}

std::shared_ptr<CardService> DESFireEV1Chip::getService(CardServiceType serviceType)
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
    case CST_STORAGE: { service.reset(new DESFireStorageCardService(shared_from_this()));
    }
    break;
    case CST_NFC_TAG:
    {
        service.reset(new DESFireEV1NFCTag4CardService(shared_from_this()));
    }
    break;
    default: break;
    }

    if (!service)
    {
        service = DESFireChip::getService(serviceType);
    }

    return service;
}

std::shared_ptr<Location> DESFireEV1Chip::createLocation() const
{
    std::shared_ptr<DESFireEV1Location> ret;
    ret.reset(new DESFireEV1Location());
    return ret;
}
}