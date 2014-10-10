/**
 * \file indalachip.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Indala chip.
 */

#include "indalachip.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>

namespace logicalaccess
{
    IndalaChip::IndalaChip()
        : Chip(CHIP_INDALA)
    {
    }

    IndalaChip::~IndalaChip()
    {
    }

    boost::shared_ptr<LocationNode> IndalaChip::getRootLocationNode()
    {
        boost::shared_ptr<LocationNode> rootNode;
        rootNode.reset(new LocationNode());

        rootNode->setName("Indala");

        return rootNode;
    }
}