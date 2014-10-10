/**
 * \file em4135chip.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief EM4135 chip.
 */

#include "em4135chip.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>

#include "logicalaccess/services/accesscontrol/formats/bithelper.hpp"

namespace logicalaccess
{
    EM4135Chip::EM4135Chip()
        : Chip("EM4135")
    {
    }

    EM4135Chip::~EM4135Chip()
    {
    }

    boost::shared_ptr<LocationNode> EM4135Chip::getRootLocationNode()
    {
        boost::shared_ptr<LocationNode> rootNode;
        rootNode.reset(new LocationNode());

        rootNode->setName("EM4135");

        return rootNode;
    }
}