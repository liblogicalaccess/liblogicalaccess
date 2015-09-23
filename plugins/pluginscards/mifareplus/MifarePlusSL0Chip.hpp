//
// Created by xaqq on 8/24/15.
//

#ifndef LIBLOGICALACCESS_MIFAREPLUSSL0CHIP_HPP
#define LIBLOGICALACCESS_MIFAREPLUSSL0CHIP_HPP

#include <string>
#include "mifarepluschip.hpp"

namespace logicalaccess
{

    class LIBLOGICALACCESS_API MifarePlusSL0Chip : public MifarePlusChip
    {
      public:
        MifarePlusSL0Chip(const std::string &cardType)
            : MifarePlusChip(cardType)
        {
        }

        virtual int getSecurityLevel() const override
        {
            return 0;
        };
    };

    class MifarePlusSL0_2kChip : public MifarePlusSL0Chip
    {
      public:
        MifarePlusSL0_2kChip()
            : Chip("MifarePlus_SL0_2K")
            , MifarePlusSL0Chip("MifarePlus_SL0_2K")
        {
        }
    };

    class MifarePlusSL0_4kChip : public MifarePlusSL0Chip
    {
      public:
        MifarePlusSL0_4kChip()
            : Chip("MifarePlus_SL0_4K")
            , MifarePlusSL0Chip("MifarePlus_SL0_4K")
        {
        }
    };
}

#endif // LIBLOGICALACCESS_MIFAREPLUSSL0CHIP_HPP
