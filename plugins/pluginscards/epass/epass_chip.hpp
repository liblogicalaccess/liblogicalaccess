#pragma once

#include "../iso7816/iso7816chip.hpp"
#include "epass_command.hpp"

namespace logicalaccess
{
class EPassChip : public ISO7816Chip
{
  public:
    EPassChip();

    std::shared_ptr<EPassCommand> getEPassCommand();

    virtual std::shared_ptr<CardService>
    getService(CardServiceType serviceType) override;
};
}
