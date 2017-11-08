/**
 * \file tagitchip.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Tag-It chip.
 */

#include <logicalaccess/plugins/cards/tagit/tagitchip.hpp>

#include <iostream>
#include <iomanip>
#include <sstream>
#include <logicalaccess/cards/locationnode.hpp>

namespace logicalaccess
{
TagItChip::TagItChip()
    : ISO15693Chip(CHIP_TAGIT)
{
}

TagItChip::~TagItChip()
{
}

std::shared_ptr<LocationNode> TagItChip::getRootLocationNode()
{
    std::shared_ptr<LocationNode> rootNode = ISO15693Chip::getRootLocationNode();
    rootNode->setName("Tag-It");

    return rootNode;
}
}