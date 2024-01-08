/**
 * \file chip.cpp
 * \author Maxime C. <maxime@leosac.com>
 * \brief Card chip.
 */

#include <logicalaccess/cards/chip.hpp>
#include <logicalaccess/cards/locationnode.hpp>
#include <fstream>

namespace logicalaccess
{
Chip::Chip()
    : d_cardtype(CHIP_UNKNOWN)
{
    d_powerStatus    = CPS_NO_POWER;
    d_receptionLevel = 0;
}

Chip::Chip(std::string cardtype)
    : d_cardtype(cardtype)
{
    d_powerStatus    = CPS_NO_POWER;
    d_receptionLevel = 0;
}

const std::string &Chip::getCardType() const
{
    return d_cardtype;
}

std::string Chip::getGenericCardType() const
{
    return d_cardtype;
}

std::shared_ptr<LocationNode> Chip::getRootLocationNode()
{
    std::shared_ptr<LocationNode> rootNode;
    rootNode.reset(new LocationNode());

    rootNode->setName("Unknown");

    return rootNode;
}

std::shared_ptr<CardService> Chip::getService(CardServiceType /*serviceType*/)
{
    return std::shared_ptr<CardService>();
}

std::shared_ptr<Location> Chip::createLocation() const
{
    return std::shared_ptr<Location>();
}

std::shared_ptr<AccessInfo> Chip::createAccessInfo() const
{
    return std::shared_ptr<AccessInfo>();
}
}