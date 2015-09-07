/**
 * \file em4102chip.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief EM4102 chip.
 */

#include "em4102chip.hpp"
#include "logicalaccess/cards/locationnode.hpp"

namespace logicalaccess
{
    EM4102Chip::EM4102Chip()
        : Chip(CHIP_EM4102)
    {
    }

    EM4102Chip::~EM4102Chip()
    {
    }

    std::shared_ptr<LocationNode> EM4102Chip::getRootLocationNode()
    {
        std::shared_ptr<LocationNode> rootNode;
        rootNode.reset(new LocationNode());

        rootNode->setName("EM4102");

        return rootNode;
    }
}