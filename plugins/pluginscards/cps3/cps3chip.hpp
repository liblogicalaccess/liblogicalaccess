/**
 * \file cps3chip.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief CPS3 chip.
 */

#ifndef LOGICALACCESS_CPS3CHIP_HPP
#define LOGICALACCESS_CPS3CHIP_HPP

#include "../iso7816/iso7816chip.hpp"
#include "cps3commands.hpp"

#include <string>
#include <vector>
#include <iostream>

namespace logicalaccess
{
#define	CHIP_CPS3	"CPS3"

    /**
     * \brief The CPS3 base chip class.
     */
    class LIBLOGICALACCESS_API CPS3Chip : public ISO7816Chip
    {
    public:

        /**
         * \brief Constructor.
         */
		CPS3Chip(std::string ct);

        /**
         * \brief Constructor.
         */
		CPS3Chip();

        /**
         * \brief Destructor.
         */
		virtual ~CPS3Chip();

        /**
         * \brief Get the generic card type.
         * \return The generic card type.
         */
        virtual std::string getGenericCardType() const { return CHIP_CPS3; };

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
		* \brief Create default CPS3 location.
		* \return Default CPS3 location.
		*/
		virtual std::shared_ptr<Location> createLocation() const;

        /**
         * \brief Get the CPS3 card provider for I/O access.
         * \return The CPS3 card provider.
         */
		std::shared_ptr<CPS3Commands> getCPS3Commands() { return std::dynamic_pointer_cast<CPS3Commands>(getCommands()); };

    protected:
    };
}

#endif