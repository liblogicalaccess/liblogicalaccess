/**
 * \file infineonmydchip.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Infineon My-D chip.
 */

#include <logicalaccess/plugins/cards/infineonmyd/infineonmydchip.hpp>

#include <iostream>
#include <iomanip>
#include <sstream>
#include <logicalaccess/cards/locationnode.hpp>

namespace logicalaccess
{
InfineonMYDChip::InfineonMYDChip()
    : ISO15693Chip(CHIP_INFINEONMYD)
{
}

InfineonMYDChip::~InfineonMYDChip()
{
}

std::shared_ptr<LocationNode> InfineonMYDChip::getRootLocationNode()
{
    std::shared_ptr<LocationNode> rootNode;
    rootNode.reset(new LocationNode());

    rootNode->setName("Infineon My-D");

    return rootNode;
}
}