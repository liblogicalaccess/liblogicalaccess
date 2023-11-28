/**
 * \file smartframechip.cpp
 * \author Maxime C. <maxime@leosac.com>
 * \brief SmartFrame chip.
 */

#include <logicalaccess/plugins/cards/smartframe/smartframechip.hpp>

#include <iostream>
#include <iomanip>
#include <sstream>
#include <logicalaccess/cards/locationnode.hpp>

namespace logicalaccess
{
SmartFrameChip::SmartFrameChip()
    : Chip(CHIP_SMARTFRAME)
{
}

SmartFrameChip::~SmartFrameChip()
{
}

std::shared_ptr<LocationNode> SmartFrameChip::getRootLocationNode()
{
    std::shared_ptr<LocationNode> rootNode;
    rootNode.reset(new LocationNode());

    rootNode->setName("SmartFrame");

    return rootNode;
}
}