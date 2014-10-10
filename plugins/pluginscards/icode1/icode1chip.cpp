/**
 * \file icode1chip.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief iCode1 chip.
 */

#include "icode1chip.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>

namespace logicalaccess
{
    ICode1Chip::ICode1Chip()
        : ISO15693Chip(CHIP_ICODE1)
    {
    }

    ICode1Chip::~ICode1Chip()
    {
    }

    boost::shared_ptr<LocationNode> ICode1Chip::getRootLocationNode()
    {
        boost::shared_ptr<LocationNode> rootNode;
        rootNode.reset(new LocationNode());

        rootNode->setName("iCode1");

        return rootNode;
    }
}