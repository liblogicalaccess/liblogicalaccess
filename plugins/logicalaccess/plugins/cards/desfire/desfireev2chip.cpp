/**
 * \file desfireEV2chip.cpp
 * \author Maxime C. <maxime@leosac.com>
 * \brief DESFire EV2 chip.
 */

#include <logicalaccess/plugins/cards/desfire/desfireev2chip.hpp>
#include <logicalaccess/plugins/cards/desfire/desfireev1location.hpp>
#include <logicalaccess/services/accesscontrol/accesscontrolcardservice.hpp>
#include <logicalaccess/plugins/cards/desfire/desfirestoragecardservice.hpp>

#include <cstring>
#include <logicalaccess/dynlibrary/librarymanager.hpp>
#include <logicalaccess/cards/locationnode.hpp>
#include <logicalaccess/plugins/cards/desfire/desfireev2crypto.hpp>

namespace logicalaccess
{
DESFireEV2Chip::DESFireEV2Chip()
    : DESFireEV1Chip(CHIP_DESFIRE_EV2)
{
    d_crypto = std::make_shared<DESFireEV2Crypto>();
}

DESFireEV2Chip::DESFireEV2Chip(const std::string &ct)
    : DESFireEV1Chip(ct)
{
    d_crypto = std::make_shared<DESFireEV2Crypto>();
}

DESFireEV2Chip::~DESFireEV2Chip()
{
}

std::shared_ptr<LocationNode> DESFireEV2Chip::getRootLocationNode()
{
    std::shared_ptr<LocationNode> rootNode = DESFireChip::getRootLocationNode();
    rootNode->setName("Mifare DESFire EV2");

    return rootNode;
}

std::shared_ptr<DESFireLocation> DESFireEV2Chip::getApplicationLocation()
{
    std::shared_ptr<DESFireEV1Location> location(new DESFireEV1Location());

    /*   try
       {
           DESFireKeySettings keySettings;
           unsigned char maxNbkeys = 0;
           DESFireKeyType keyType;
           getDESFireEV2Commands()->getKeySettings(keySettings, maxNbkeys, keyType);

           if (keyType != DF_KEY_DES)
           {
               location->useEV1 = true;
               location->cryptoMethod = keyType;
           }
       }
       catch (std::exception&)
       {
       }*/

    return location;
}

std::shared_ptr<CardService> DESFireEV2Chip::getService(CardServiceType serviceType)
{
    std::shared_ptr<CardService> service;

    switch (serviceType)
    {
    case CST_ACCESS_CONTROL:
    {
        service = LibraryManager::getInstance()->getCardService(
            shared_from_this(), CardServiceType::CST_ACCESS_CONTROL);
        if (!service)
            service.reset(new AccessControlCardService(shared_from_this()));
    }
    break;
    case CST_STORAGE: { service.reset(new DESFireStorageCardService(shared_from_this()));
    }
    break;
    default: break;
    }

    if (!service)
    {
        service = DESFireEV1Chip::getService(serviceType);
    }

    return service;
}

std::shared_ptr<Location> DESFireEV2Chip::createLocation() const
{
    std::shared_ptr<DESFireEV1Location> ret;
    ret.reset(new DESFireEV1Location());
    return ret;
}
} // namespace logicalaccess