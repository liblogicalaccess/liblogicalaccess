//
// Created by xaqq on 8/24/15.
//

#ifndef LIBLOGICALACCESS_MIFAREPLUSSL0CHIP_HPP
#define LIBLOGICALACCESS_MIFAREPLUSSL0CHIP_HPP

#include <string>
#include "mifarepluschip.hpp"

namespace logicalaccess
{

class LIBLOGICALACCESS_API MifarePlusSL0Chip : public MifarePlusChip, public Chip
{
  public:
    explicit MifarePlusSL0Chip(const std::string &cardType)
        : Chip(cardType)
    {
    }

    int getSecurityLevel() const override
    {
        return 0;
    }

    const std::string &getCardType() const override
    {
        return Chip::getCardType();
    }

    std::string getGenericCardType() const override
    {
        return Chip::getGenericCardType();
    }

    std::shared_ptr<Commands> getCommands() const override
    {
        return Chip::getCommands();
    }

    ByteVector getChipIdentifier() const override
    {
        return Chip::getChipIdentifier();
    }

    std::shared_ptr<CardService> getService(CardServiceType serviceType) override
    {
        return Chip::getService(serviceType);
    }
};

class MifarePlusSL0_2kChip : public MifarePlusSL0Chip
{
  public:
    MifarePlusSL0_2kChip()
        : MifarePlusSL0Chip("MifarePlus_SL0_2K")
    {
    }
};

class MifarePlusSL0_4kChip : public MifarePlusSL0Chip
{
  public:
    MifarePlusSL0_4kChip()
        : MifarePlusSL0Chip("MifarePlus_SL0_4K")
    {
    }
};
}

#endif // LIBLOGICALACCESS_MIFAREPLUSSL0CHIP_HPP
