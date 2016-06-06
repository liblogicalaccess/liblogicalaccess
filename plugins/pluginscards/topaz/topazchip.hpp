/**
 * \file topazchip.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Topaz chip.
 */

#ifndef LOGICALACCESS_TOPAZCHIP_H
#define LOGICALACCESS_TOPAZCHIP_H

#include "logicalaccess/cards/chip.hpp"

#include "topazcommands.hpp"
#include "topazprofile.hpp"

#include <string>
#include <vector>
#include <iostream>

namespace logicalaccess
{
#define CHIP_TOPAZ		"Topaz"

    /**
     * \brief The Topaz base chip class.
     */
    class LIBLOGICALACCESS_API TopazChip : public Chip
    {
    public:

        /**
         * \brief Constructor.
         */
        TopazChip();

        /**
         * \brief Constructor.
         * \param ct The card type.
         */
        TopazChip(std::string ct);

        /**
         * \brief Destructor.
         */
        virtual ~TopazChip();

        /**
         * \brief Get the generic card type.
         * \return The generic card type.
         */
        virtual std::string getGenericCardType() const { return CHIP_TOPAZ; };

        /**
         * \brief Get the root location node.
         * \return The root location node.
         */
        virtual std::shared_ptr<LocationNode> getRootLocationNode();

        /**
         * \brief Get a card service for this chip.
         * \param serviceType The card service type.
         * \return The card service.
         */
        virtual std::shared_ptr<CardService> getService(CardServiceType serviceType);

		/**
		 * \brief Get the number of blocks.
		 * \return Number of blocks.
		 */
		virtual unsigned short getNbBlocks(bool checkOnCard = false);

        /**
         * \brief Get the Topaz commands.
         * \return The Topaz commands.
         */
        std::shared_ptr<TopazCommands> getTopazCommands() { return std::dynamic_pointer_cast<TopazCommands>(getCommands()); };

        /**
         * \brief Get the Topaz profile.
         * \return The Topaz profile.
         */
        std::shared_ptr<TopazProfile> getTopazProfile() { return std::dynamic_pointer_cast<TopazProfile>(getProfile()); };

    protected:

        void addPageNode(std::shared_ptr<LocationNode> rootNode, int page);

        void checkRootLocationNodeName(std::shared_ptr<LocationNode> rootNode);

		unsigned short d_nbblocks;
    };
}

#endif