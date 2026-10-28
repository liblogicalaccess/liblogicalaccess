#include <logicalaccess/plugins/cards/duox/duoxchip.hpp>
#include <logicalaccess/plugins/cards/duox/duoxcrypto.hpp>
#include <logicalaccess/cards/locationnode.hpp>

namespace logicalaccess
{

DUOXChip::DUOXChip()
    : DESFireEV3Chip(CHIP_DUOX)
{
    d_crypto = std::make_shared<DUOXCrypto>();
}

std::shared_ptr<LocationNode> DUOXChip::getRootLocationNode()
{
    auto rootNode = DESFireEV3Chip::getRootLocationNode();
    rootNode->setName("MIFARE DUOX");

    return rootNode;
}

} // namespace logicalaccess