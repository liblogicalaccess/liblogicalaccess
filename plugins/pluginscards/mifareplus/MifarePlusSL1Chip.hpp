//
// Created by xaqq on 7/1/15.
//

#ifndef LIBLOGICALACCESS_MIFAREPLUSSL1CHIP_H
#define LIBLOGICALACCESS_MIFAREPLUSSL1CHIP_H


#include "../mifare/mifarechip.hpp"
#include "mifarepluschip.hpp"

namespace logicalaccess
{

    /**
     * A base class for MifarePlus in SecurityLevel 1.
     *
     * This mostly acts as a tag, because the chip has no additional
     * functions.
     */
    class LIBLOGICALACCESS_API MifarePlusSL1Chip : public MifarePlusChip,
                                                   public MifareChip
    {
      public:
        using MifarePlusChip::getCardType;
        MifarePlusSL1Chip(const std::string &cardType, int nb_sectors);

        virtual std::shared_ptr<CardService>
        getService(CardServiceType serviceType) override;

        virtual int getSecurityLevel() const override;

        virtual std::string getGenericCardType() const override
        {
            return MifarePlusChip::getGenericCardType();
        }
    };

    class MifarePlusSL1_2kChip : public MifarePlusSL1Chip
    {
      public:
        MifarePlusSL1_2kChip();
    };

    class MifarePlusSL1_4kChip : public MifarePlusSL1Chip
    {
      public:
        MifarePlusSL1_4kChip();

        /**
         * \brief Get the root location node.
         * \return The root location node.
         */
        virtual std::shared_ptr<LocationNode> getRootLocationNode();
    };
}

#endif // LIBLOGICALACCESS_MIFAREPLUSSL1CHIP_H
