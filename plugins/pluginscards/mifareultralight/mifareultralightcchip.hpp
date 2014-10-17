/**
 * \file mifareultralightchip.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Mifare Ultralight chip.
 */

#ifndef LOGICALACCESS_MIFAREULTRALIGHTCCHIP_HPP
#define LOGICALACCESS_MIFAREULTRALIGHTCCHIP_HPP

#include "mifareultralightchip.hpp"

#include "mifareultralightccommands.hpp"
#include "mifareultralightcprofile.hpp"

#include <string>
#include <vector>
#include <iostream>

namespace logicalaccess
{
#define CHIP_MIFAREULTRALIGHTC		"MifareUltralightC"

    /**
     * \brief The Mifare Ultralight C base chip class.
     */
    class LIBLOGICALACCESS_API MifareUltralightCChip : public MifareUltralightChip
    {
    public:

        /**
         * \brief Constructor.
         */
        MifareUltralightCChip();

        /**
         * \brief Destructor.
         */
        virtual ~MifareUltralightCChip();

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
         * \brief Get the Mifare Ultralight C card provider for I/O access.
         * \return The Mifare Ultralight C card provider.
         */
        std::shared_ptr<MifareUltralightCCommands> getMifareUltralightCCommands() { return std::dynamic_pointer_cast<MifareUltralightCCommands>(getMifareUltralightCommands()); };

        /**
         * \brief Get the Mifare Ultralight C profile.
         * \return The Mifare Ultralight C profile.
         */
        std::shared_ptr<MifareUltralightCProfile> getMifareUltralightCProfile() { return std::dynamic_pointer_cast<MifareUltralightCProfile>(getMifareUltralightProfile()); };

    protected:
    };
}

#endif