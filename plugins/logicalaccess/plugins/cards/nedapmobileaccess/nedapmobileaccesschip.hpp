#pragma once

#include <logicalaccess/plugins/cards/iso7816/iso7816chip.hpp>
#include <logicalaccess/plugins/cards/nedapmobileaccess/nedapmobileaccesscommands.hpp>
#include <logicalaccess/plugins/cards/nedapmobileaccess/lla_cards_nedapmobileaccess_api.hpp>

namespace logicalaccess
{
#define CHIP_NEDAP_MOBILE_ACCESS "NedapMobileAccess"

class LLA_CARDS_NEDAPMOBILEACCESS_API NedapMobileAccessChip : public ISO7816Chip
{
  public:
    NedapMobileAccessChip();

    std::string getGenericCardType() const override
    {
        return CHIP_NEDAP_MOBILE_ACCESS;
    }

    std::shared_ptr<NedapMobileAccessCommands> getNedapMobileAccessCommands() const
    {
        return std::dynamic_pointer_cast<NedapMobileAccessCommands>(getCommands());
    }

    std::shared_ptr<ISO7816Commands> getISO7816Commands() const override
    {
        auto commands = getNedapMobileAccessCommands();
        return commands ? commands->getISO7816Commands()
                        : ISO7816Chip::getISO7816Commands();
    }
};
}
