/**
 * \file mifareplusxchip.hpp
 * \author Xavier S. <xavier.schmerber@gmail.com>
 * \brief MifarePlusX chip.
 */

#ifndef LOGICALACCESS_MIFAREPLUSXCHIP_HPP
#define LOGICALACCESS_MIFAREPLUSXCHIP_HPP

#include "logicalaccess/cards/chip.hpp"
#include "../mifare/mifareaccessinfo.hpp"

#include <string>
#include <vector>
#include <iostream>

namespace logicalaccess
{
#define	CHIP_MIFAREPLUSX			"MifarePlusX"
	
    class LIBLOGICALACCESS_API MifarePlusXChip : public MifarePlusChip, public Chip
    {
    public:
        ~MifarePlusXChip() = default;
        MifarePlusXChip() : Chip(CHIP_MIFAREPLUSX) { };


		const std::string& getCardType() const override { return Chip::getCardType(); }

		std::string getGenericCardType() const override { return Chip::getGenericCardType(); }

		std::shared_ptr<Commands> getCommands() const override { return Chip::getCommands(); }

		const std::vector<unsigned char> getChipIdentifier() const override { return Chip::getChipIdentifier(); }

		std::shared_ptr<CardService> getService(CardServiceType serviceType) override { return Chip::getService(serviceType); }
    };
}

#endif /* LOGICALACCESS_MIFAREPLUSXCHIP_HPP */