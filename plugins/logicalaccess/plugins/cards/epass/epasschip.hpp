#pragma once

#include <logicalaccess/plugins/cards/iso7816/iso7816chip.hpp>
#include <logicalaccess/plugins/cards/epass/epasscommands.hpp>

namespace logicalaccess
{
#define CHIP_EPASS "EPass"

class LIBLOGICALACCESS_API EPassChip : public ISO7816Chip
{
  public:
    EPassChip();
    virtual ~EPassChip() = default;

    std::shared_ptr<EPassCommands> getEPassCommands() const;

    std::shared_ptr<CardService> getService(CardServiceType serviceType) override;

    /**
     * \brief Create default EPass access information.
     * \return Default EPass access information.
     */
    std::shared_ptr<AccessInfo> createAccessInfo() const override;
};
}
