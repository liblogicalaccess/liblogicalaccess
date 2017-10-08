#pragma once

#include "../mifare/mifarechip.hpp"

namespace logicalaccess
{
    class MifarePlusSL3Chip : public MifarePlusChip, public Chip
    {
    public:
        MifarePlusSL3Chip(int is_2k) :
                Chip(is_2k ? "MifarePlus_SL3_2K" : "MifarePlus_SL3_4K")
        {

        }

        int getSecurityLevel() const override { return 3;};

		const std::string& getCardType() const override { return Chip::getCardType(); }

		std::string getGenericCardType() const override { return Chip::getGenericCardType(); }

		std::shared_ptr<Commands> getCommands() const override { return Chip::getCommands(); }

		const std::vector<unsigned char> getChipIdentifier() const override { return Chip::getChipIdentifier(); }

		std::shared_ptr<CardService> getService(CardServiceType serviceType) override { return Chip::getService(serviceType); }
    };
}
