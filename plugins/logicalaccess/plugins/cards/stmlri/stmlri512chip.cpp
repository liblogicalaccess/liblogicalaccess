/**
 * \file stmlri512chip.cpp
 * \author Maxime C. <maxime@leosac.com>
 * \brief StmLri512 chip.
 */

#include <logicalaccess/plugins/cards/stmlri/stmlri512chip.hpp>

#include <iostream>
#include <iomanip>
#include <sstream>
#include <logicalaccess/cards/locationnode.hpp>

namespace logicalaccess
{
StmLri512Chip::StmLri512Chip()
    : Chip(CHIP_STMLRI512)
{
}

StmLri512Chip::~StmLri512Chip()
{
}

std::shared_ptr<LocationNode> StmLri512Chip::getRootLocationNode()
{
    std::shared_ptr<LocationNode> rootNode;
    rootNode.reset(new LocationNode());

    rootNode->setName("STM LRI512");

    return rootNode;
}
}