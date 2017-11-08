/**
 * \file proxlitechip.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief ProxLite chip.
 */

#include <logicalaccess/plugins/cards/proxlite/proxlitechip.hpp>

#include <iostream>
#include <iomanip>
#include <sstream>
#include <logicalaccess/cards/locationnode.hpp>

namespace logicalaccess
{
ProxLiteChip::ProxLiteChip()
    : Chip(CHIP_PROXLITE)
{
}

ProxLiteChip::~ProxLiteChip()
{
}

std::shared_ptr<LocationNode> ProxLiteChip::getRootLocationNode()
{
    std::shared_ptr<LocationNode> rootNode;
    rootNode.reset(new LocationNode());

    rootNode->setName("ProxLite");

    return rootNode;
}
}