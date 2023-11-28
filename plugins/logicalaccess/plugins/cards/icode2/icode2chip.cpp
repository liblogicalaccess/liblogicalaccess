/**
 * \file icode2chip.cpp
 * \author Maxime C. <maxime@leosac.com>
 * \brief iCode2 chip.
 */

#include <logicalaccess/plugins/cards/icode2/icode2chip.hpp>

#include <iostream>
#include <iomanip>
#include <sstream>
#include <logicalaccess/cards/locationnode.hpp>

namespace logicalaccess
{
ICode2Chip::ICode2Chip()
    : ISO15693Chip(CHIP_ICODE2)
{
}

ICode2Chip::~ICode2Chip()
{
}

std::shared_ptr<LocationNode> ICode2Chip::getRootLocationNode()
{
    std::shared_ptr<LocationNode> rootNode;
    rootNode.reset(new LocationNode());

    rootNode->setName("iCode2");

    return rootNode;
}
}