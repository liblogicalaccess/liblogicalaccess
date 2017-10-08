/**
 * \file mifareultralightchip.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Mifare Ultralight chip.
 */

#ifndef LOGICALACCESS_MIFAREULTRALIGHTCCHIP_HPP
#define LOGICALACCESS_MIFAREULTRALIGHTCCHIP_HPP

#include "mifareultralightchip.hpp"

#include "mifareultralightccommands.hpp"

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
	    std::shared_ptr<LocationNode> getRootLocationNode() override;

        /**
         * \brief Get a card service for this chip.
         * \param serviceType The card service type.
         * \return The card service.
         */
	    std::shared_ptr<CardService> getService(CardServiceType serviceType) override;

		/**
		* \brief Create default access informations.
		* \return Default access informations. Always null.
		*/
	    std::shared_ptr<AccessInfo> createAccessInfo() const override;

		std::shared_ptr<TripleDESKey> getDefaultKey() const;

        /**
         * \brief Get the Mifare Ultralight C card provider for I/O access.
         * \return The Mifare Ultralight C card provider.
         */
        std::shared_ptr<MifareUltralightCCommands> getMifareUltralightCCommands() const { return std::dynamic_pointer_cast<MifareUltralightCCommands>(getMifareUltralightCommands()); }
    };
}

#endif