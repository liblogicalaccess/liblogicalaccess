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
    class LIBLOGICALACCESS_API MifarePlusSL1Chip : public MifareChip, public MifarePlusChip
    {
      public:
        MifarePlusSL1Chip(const std::string &cardType, int nb_sectors);

	    int getSecurityLevel() const override;

	    std::shared_ptr<AccessInfo> createAccessInfo() const override;

		const std::string& getCardType() const override { return MifareChip::getCardType(); }

		std::string getGenericCardType() const override { return MifareChip::getGenericCardType(); }

		std::shared_ptr<Commands> getCommands() const override { return MifareChip::getCommands(); }

		ByteVector getChipIdentifier() const override { return MifareChip::getChipIdentifier(); }

		std::shared_ptr<CardService> getService(CardServiceType serviceType) override;
    };

	class LIBLOGICALACCESS_API MifarePlusSL1_2kChip : public MifarePlusSL1Chip
    {
      public:
        MifarePlusSL1_2kChip();
    };

	class LIBLOGICALACCESS_API MifarePlusSL1_4kChip : public MifarePlusSL1Chip
    {
      public:
        MifarePlusSL1_4kChip();

        /**
         * \brief Get the root location node.
         * \return The root location node.
         */
		std::shared_ptr<LocationNode> getRootLocationNode() override;
    };
}

#endif // LIBLOGICALACCESS_MIFAREPLUSSL1CHIP_H
