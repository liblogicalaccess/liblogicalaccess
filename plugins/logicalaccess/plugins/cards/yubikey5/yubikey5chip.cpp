/**
 * \file yubikey5chip.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Yubikey5 chip.
 */

#include <logicalaccess/plugins/cards/yubikey5/yubikey5chip.hpp>

#include <iostream>
#include <iomanip>
#include <sstream>

#include <logicalaccess/services/accesscontrol/formats/bithelper.hpp>
#include <logicalaccess/cards/locationnode.hpp>

namespace logicalaccess
{
Yubikey5Chip::Yubikey5Chip(std::string ct)
    : ISO7816Chip(ct)
{
}

Yubikey5Chip::Yubikey5Chip()
    : ISO7816Chip(CHIP_YUBIKEY5)
{
}

Yubikey5Chip::~Yubikey5Chip()
{
}

/*std::shared_ptr<CardService> Yubikey5Chip::getService(CardServiceType serviceType)
{
    std::shared_ptr<CardService> service;

    switch (serviceType)
    {
    default: break;
    }

    return service;
}*/
}