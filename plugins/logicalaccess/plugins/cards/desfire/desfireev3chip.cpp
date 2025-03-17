
#include <logicalaccess/plugins/cards/desfire/desfireev3chip.hpp>
#include <logicalaccess/cards/locationnode.hpp>
#include <logicalaccess/plugins/cards/desfire/desfireev3nfctag4cardservice.hpp>

namespace logicalaccess
{
DESFireEV3Chip::DESFireEV3Chip()
    : DESFireEV2Chip(CHIP_DESFIRE_EV3)
{
}

std::shared_ptr<LocationNode> DESFireEV3Chip::getRootLocationNode()
{
    std::shared_ptr<LocationNode> rootNode = DESFireEV2Chip::getRootLocationNode();
    rootNode->setName("Mifare DESFire EV3");

    return rootNode;
}

std::shared_ptr<CardService> DESFireEV3Chip::getService(CardServiceType serviceType)
{
    std::shared_ptr<CardService> service;
    switch (serviceType)
    {
        case CST_NFC_TAG:
        {
            service.reset(new DESFireEV3NFCTag4CardService(shared_from_this()));
        }
        break;
        default:
        {
            service = DESFireEV2Chip::getService(serviceType);
        }
        break;
    }
    return service;
}
}