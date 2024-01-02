
#include <logicalaccess/plugins/cards/desfire/desfireev3chip.hpp>
#include <logicalaccess/cards/locationnode.hpp>

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

}
