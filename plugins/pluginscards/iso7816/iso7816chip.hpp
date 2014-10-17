/**
 * \file iso7816chip.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief ISO7816 chip.
 */

#ifndef LOGICALACCESS_ISO7816CHIP_HPP
#define LOGICALACCESS_ISO7816CHIP_HPP

#include "logicalaccess/cards/chip.hpp"
#include "iso7816commands.hpp"
#include "iso7816profile.hpp"

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
        ISO7816Chip(std::string ct);

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
        virtual std::string getGenericCardType() const { return CHIP_ISO7816; };

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
         * \brief Get the ISO7816 commands.
         * \return The ISO7816 commands.
         */
        std::shared_ptr<ISO7816Commands> getISO7816Commands() { return std::dynamic_pointer_cast<ISO7816Commands>(getCommands()); };

        /**
         * \brief Get the ISO7816 profile.
         * \return The ISO7816 profile.
         */
        std::shared_ptr<ISO7816Profile> getISO7816Profile() { return std::dynamic_pointer_cast<ISO7816Profile>(getProfile()); };

    protected:
    };
}

#endif