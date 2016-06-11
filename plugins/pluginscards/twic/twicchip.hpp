/**
 * \file twicchip.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Twic chip.
 */

#ifndef LOGICALACCESS_TWICCHIP_HPP
#define LOGICALACCESS_TWICCHIP_HPP

#include "../iso7816/iso7816chip.hpp"
#include "twiccommands.hpp"

#include <string>
#include <vector>
#include <iostream>

namespace logicalaccess
{
#define CHIP_TWIC		"Twic"

    /**
     * \brief The Twic base chip class.
     */
    class LIBLOGICALACCESS_API TwicChip : public ISO7816Chip
    {
    public:

        /**
         * \brief Constructor.
         */
        TwicChip();

        /**
         * \brief Destructor.
         */
        virtual ~TwicChip();

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
		* \brief Create default Twic location.
		* \return Default Twic location.
		*/
		virtual std::shared_ptr<Location> createLocation() const;

        /**
         * \brief Get the Twic commands.
         * \return The Twic commands.
         */
        std::shared_ptr<TwicCommands> getTwicCommands() { return std::dynamic_pointer_cast<TwicCommands>(getISO7816Commands()); };

    protected:
    };
}

#endif