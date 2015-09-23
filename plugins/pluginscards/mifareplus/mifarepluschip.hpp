/**
 * \file mifarechip.hpp
 * \author Xavier S. <xavier.schmerber@gmail.com>
 * \brief MifarePlus chip.
 */

#ifndef LOGICALACCESS_MIFAREPLUSCHIP_HPP
#define LOGICALACCESS_MIFAREPLUSCHIP_HPP

#include "logicalaccess/cards/chip.hpp"
#include "mifareplussl1profile.hpp"

#include <string>
#include <vector>
#include <iostream>

namespace logicalaccess
{
#define	CHIP_MIFAREPLUS4K			"MifarePlus4K"
#define	CHIP_MIFAREPLUS2K			"MifarePlus2K"

    class LIBLOGICALACCESS_API MifarePlusChip : public virtual Chip
    {
    public:
        ~MifarePlusChip() = default;
        MifarePlusChip() { };
        MifarePlusChip(const std::string &cardtype) :
                Chip(cardtype) {};
        using Chip::shared_from_this;
        virtual int getSecurityLevel() const { return -1 ; } ;

        /**
         * Returns the AES key number on the card for a given sector and key type.
         */
        static uint16_t key_number_from_sector(int sector, MifareKeyType type);

		virtual std::string getGenericCardType() const override;
    };
}

#endif /* LOGICALACCESS_MIFAREPLUSCHIP_HPP */