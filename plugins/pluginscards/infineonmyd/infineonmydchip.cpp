/**
 * \file infineonmydchip.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Infineon My-D chip.
 */

#include "infineonmydchip.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>

namespace logicalaccess
{
    InfineonMYDChip::InfineonMYDChip()
        : ISO15693Chip(CHIP_INFINEONMYD)
    {
    }

    InfineonMYDChip::~InfineonMYDChip()
    {
    }

    boost::shared_ptr<LocationNode> InfineonMYDChip::getRootLocationNode()
    {
        boost::shared_ptr<LocationNode> rootNode;
        rootNode.reset(new LocationNode());

        rootNode->setName("Infineon My-D");

        return rootNode;
    }
}