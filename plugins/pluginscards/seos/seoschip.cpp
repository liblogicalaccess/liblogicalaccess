/**
 * \file seoschip.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief SEOS chip.
 */

#include "seoschip.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>
#include <logicalaccess/dynlibrary/librarymanager.hpp>
#include <cassert>

#include "logicalaccess/cards/locationnode.hpp"

namespace logicalaccess
{
SEOSChip::SEOSChip()
    : Chip(CHIP_SEOS)
{
}

SEOSChip::~SEOSChip()
{
}

std::shared_ptr<LocationNode> SEOSChip::getRootLocationNode()
{
    std::shared_ptr<LocationNode> rootNode;
    rootNode.reset(new LocationNode());

    rootNode->setName("HID SEOS");

    return rootNode;
}
}