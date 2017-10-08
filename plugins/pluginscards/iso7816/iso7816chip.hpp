/**
 * \file iso7816chip.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief ISO7816 chip.
 */

#ifndef LOGICALACCESS_ISO7816CHIP_HPP
#define LOGICALACCESS_ISO7816CHIP_HPP

#include "logicalaccess/cards/chip.hpp"
#include "iso7816commands.hpp"

#include <string>
#include <vector>
#include <iostream>

namespace logicalaccess
{
#define CHIP_ISO7816	"ISO7816"

    /**
     * \brief The ISO7816 base chip class.
     */
    class LIBLOGICALACCESS_API ISO7816Chip : public Chip
    {
    public:

        /**
         * \brief Constructor.
         */
	    explicit ISO7816Chip(std::string ct);

        /**
         * \brief Constructor.
         */
        ISO7816Chip();

        /**
         * \brief Destructor.
         */
        virtual ~ISO7816Chip();

        /**
         * \brief Get the generic card type.
         * \return The generic card type.
         */
	    std::string getGenericCardType() const override { return CHIP_ISO7816; }

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
		* \brief Create default location.
		* \return Default location.
		*/
	    std::shared_ptr<Location> createLocation() const override;

        /**
         * \brief Get the ISO7816 commands.
         * \return The ISO7816 commands.
         */
        std::shared_ptr<ISO7816Commands> getISO7816Commands() const { return std::dynamic_pointer_cast<ISO7816Commands>(getCommands()); }
    };
}

#endif