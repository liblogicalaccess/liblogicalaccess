/**
 * \file icode2chip.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief iCode2 chip.
 */

#include "icode2chip.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>

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