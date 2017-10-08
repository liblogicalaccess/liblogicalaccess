/**
 * \file mifareultralightchip.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Mifare Ultralight chip.
 */

#ifndef LOGICALACCESS_MIFAREULTRALIGHTCHIP_H
#define LOGICALACCESS_MIFAREULTRALIGHTCHIP_H

#include "logicalaccess/cards/chip.hpp"

#include "mifareultralightcommands.hpp"

#include <string>
#include <vector>
#include <iostream>

namespace logicalaccess
{
#define CHIP_MIFAREULTRALIGHT		"MifareUltralight"

    /**
     * \brief The Mifare Ultralight base chip class.
     */
    class LIBLOGICALACCESS_API MifareUltralightChip : public Chip
    {
    public:

        /**
         * \brief Constructor.
         */
        MifareUltralightChip();

        /**
         * \brief Constructor.
         * \param ct The card type.
         */
	    explicit MifareUltralightChip(std::string ct);

        /**
         * \brief Destructor.
         */
        virtual ~MifareUltralightChip();

        /**
         * \brief Get the generic card type.
         * \return The generic card type.
         */
	    std::string getGenericCardType() const override { return CHIP_MIFAREULTRALIGHT; }

        /**
         * \brief Get the root location node.
         * \return The root location node.
         */
	    std::shared_ptr<LocationNode> getRootLocationNode() override;

        /**
         * \brief Get a card service for this chip.
         * \param serviceType The card service type.
         * \return The card service.
         */
	    std::shared_ptr<CardService> getService(CardServiceType serviceType) override;

		/**
		 * \brief Get the number of blocks.
		 * \return Number of blocks.
		 */
		virtual unsigned short getNbBlocks(bool checkOnCard = false);

		/**
		* \brief Create default access informations.
		* \return Default access informations. Always null.
		*/
	    std::shared_ptr<AccessInfo> createAccessInfo() const override;

		/**
		* \brief Create default location.
		* \return Default location.
		*/
	    std::shared_ptr<Location> createLocation() const override;

        /**
         * \brief Get the Mifare Ultralight commands.
         * \return The Mifare Ultralight commands.
         */
        std::shared_ptr<MifareUltralightCommands> getMifareUltralightCommands() const { return std::dynamic_pointer_cast<MifareUltralightCommands>(getCommands()); }

    protected:

		static void addBlockNode(std::shared_ptr<LocationNode> rootNode, int block);

        void checkRootLocationNodeName(std::shared_ptr<LocationNode> rootNode);

		unsigned short d_nbblocks;
    };
}

#endif