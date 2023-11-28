/**
 * \file yubikeychip.cpp
 * \author Maxime C. <maxime@leosac.com>
 * \brief Yubikey chip.
 */

#include <logicalaccess/plugins/cards/yubikey/yubikeychip.hpp>
#include <logicalaccess/plugins/cards/yubikey/yubikeychallengecardservice.hpp>

namespace logicalaccess
{
YubikeyChip::YubikeyChip(std::string ct)
    : Chip(ct)
{
}

YubikeyChip::YubikeyChip()
    : Chip(CHIP_YUBIKEY)
{
}

YubikeyChip::~YubikeyChip()
{
}

std::shared_ptr<CardService> YubikeyChip::getService(CardServiceType serviceType)
{
    std::shared_ptr<CardService> service;

    switch (serviceType)
    {
		case CST_CHALLENGE_RESPONSE:
            service.reset(new YubikeyChallengeCardService(shared_from_this()));
            break;
        default: break;
    }
    
    if (!service)
    {
        service = Chip::getService(serviceType);
    }

    return service;
}
}