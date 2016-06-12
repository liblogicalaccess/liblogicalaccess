/**
 * \file topazchip.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Topaz chip.
 */

#ifndef LOGICALACCESS_TOPAZCHIP_H
#define LOGICALACCESS_TOPAZCHIP_H

#include "logicalaccess/cards/chip.hpp"

#include "topazcommands.hpp"

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
		* \brief Create default access informations.
		* \return Default access informations. Always null.
		*/
		virtual std::shared_ptr<AccessInfo> createAccessInfo() const;

		/**
		* \brief Create default location.
		* \return Default location.
		*/
		virtual std::shared_ptr<Location> createLocation() const;

        /**
         * \brief Get the Topaz commands.
         * \return The Topaz commands.
         */
        std::shared_ptr<TopazCommands> getTopazCommands() { return std::dynamic_pointer_cast<TopazCommands>(getCommands()); };

    protected:

        void addBlockNode(std::shared_ptr<LocationNode> rootNode, int block);

        void checkRootLocationNodeName(std::shared_ptr<LocationNode> rootNode);

		unsigned short d_nbblocks;
    };
}

#endif