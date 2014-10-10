/**
 * \file desfireev1chip.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief DESFire EV1 chip.
 */

#include "desfireev1chip.hpp"
#include "logicalaccess/services/accesscontrol/accesscontrolcardservice.hpp"
#include "desfirestoragecardservice.hpp"
#include "desfireev1nfctag4cardservice.hpp"

#include <cstring>

namespace logicalaccess
{
    DESFireEV1Chip::DESFireEV1Chip() :
        DESFireChip(CHIP_DESFIRE_EV1)
    {
        d_profile.reset(new DESFireEV1Profile());
    }

    DESFireEV1Chip::~DESFireEV1Chip()
    {
    }

    boost::shared_ptr<LocationNode> DESFireEV1Chip::getRootLocationNode()
    {
        boost::shared_ptr<LocationNode> rootNode = DESFireChip::getRootLocationNode();
        rootNode->setName("Mifare DESFire EV1");

        return rootNode;
    }

    boost::shared_ptr<DESFireLocation> DESFireEV1Chip::getApplicationLocation()
    {
        boost::shared_ptr<DESFireEV1Location> location(new DESFireEV1Location());

        try
        {
            DESFireKeySettings keySettings;
            unsigned char maxNbkeys = 0;
            DESFireKeyType keyType;
            getDESFireEV1Commands()->getKeySettings(keySettings, maxNbkeys, keyType);

            if (keyType != DF_KEY_DES)
            {
                location->useEV1 = true;
                location->cryptoMethod = keyType;
            }
        }
        catch (std::exception&)
        {
        }

        return location;
    }

    boost::shared_ptr<CardService> DESFireEV1Chip::getService(CardServiceType serviceType)
    {
        boost::shared_ptr<CardService> service;

        switch (serviceType)
        {
        case CST_ACCESS_CONTROL:
        {
            service.reset(new AccessControlCardService(shared_from_this()));
        }
            break;
        case CST_STORAGE:
        {
            service.reset(new DESFireStorageCardService(shared_from_this()));
        }
            break;
        case CST_NFC_TAG:
        {
            service.reset(new DESFireEV1NFCTag4CardService(shared_from_this()));
        }
            break;
        }

        if (!service)
        {
            service = DESFireChip::getService(serviceType);
        }

        return service;
    }
}