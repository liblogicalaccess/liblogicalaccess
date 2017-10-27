/**
 * \file legicprimechip.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Legic Prime chip.
 */

#include "legicprimechip.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>
#include "logicalaccess/cards/locationnode.hpp"

namespace logicalaccess
{
LegicPrimeChip::LegicPrimeChip()
    : Chip(CHIP_LEGICPRIME)
{
}

LegicPrimeChip::~LegicPrimeChip()
{
}

std::shared_ptr<LocationNode> LegicPrimeChip::getRootLocationNode()
{
    std::shared_ptr<LocationNode> rootNode;
    rootNode.reset(new LocationNode());

    rootNode->setName("Legic Prime");

    return rootNode;
}
}