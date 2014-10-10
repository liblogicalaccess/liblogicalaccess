/**
 * \file chip.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Card chip.
 */

#include "logicalaccess/cards/chip.hpp"

#include <fstream>

using std::ofstream;
using std::ifstream;

namespace logicalaccess
{
    Chip::Chip() : d_cardtype("UNKNOWN")
    {
        d_powerStatus = CPS_NO_POWER;
        d_receptionLevel = 0;
    }

    Chip::Chip(std::string cardtype) : d_cardtype(cardtype)
    {
        d_powerStatus = CPS_NO_POWER;
        d_receptionLevel = 0;
    }

    const std::string& Chip::getCardType() const
    {
        return d_cardtype;
    }

    std::string Chip::getGenericCardType() const
    {
        return d_cardtype;
    }

    boost::shared_ptr<LocationNode> Chip::getRootLocationNode()
    {
        boost::shared_ptr<LocationNode> rootNode;
        rootNode.reset(new LocationNode());

        rootNode->setName("Unknown");

        return rootNode;
    }

    boost::shared_ptr<CardService> Chip::getService(CardServiceType /*serviceType*/)
    {
        return boost::shared_ptr<CardService>();
    }
}