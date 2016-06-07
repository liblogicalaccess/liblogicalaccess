/**
 * \file felicachip.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief FeliCa chip.
 */

#ifndef LOGICALACCESS_FELICACHIP_HPP
#define LOGICALACCESS_FELICACHIP_HPP

#include "logicalaccess/cards/chip.hpp"
#include "felicacommands.hpp"
#include "felicaprofile.hpp"

#include <string>
#include <vector>
#include <iostream>

namespace logicalaccess
{
#define	CHIP_FELICA	"FeliCa"

#define FELICA_CODE_NDEF_READ 0x000B
#define FELICA_CODE_NDEF_WRITE 0x0009

    /**
     * \brief The FeliCa base chip class.
     */
	class LIBLOGICALACCESS_API FeliCaChip : public Chip
    {
    public:

        /**
         * \brief Constructor.
         */
		FeliCaChip(std::string ct);

        /**
         * \brief Constructor.
         */
		FeliCaChip();

        /**
         * \brief Destructor.
         */
		virtual ~FeliCaChip();

        /**
         * \brief Get the generic card type.
         * \return The generic card type.
         */
        virtual std::string getGenericCardType() const { return CHIP_FELICA; };

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
         * \brief Get the FeliCa card provider for I/O access.
         * \return The FeliCa card provider.
         */
		std::shared_ptr<FeliCaCommands> getFeliCaCommands() { return std::dynamic_pointer_cast<FeliCaCommands>(getCommands()); };

        /**
         * \brief Get the FeliCa profile.
         * \return The FeliCa profile.
         */
		std::shared_ptr<FeliCaProfile> getFeliCaProfile() { return std::dynamic_pointer_cast<FeliCaProfile>(getProfile()); };

    protected:

        void addBlockNode(std::shared_ptr<LocationNode> rootNode, unsigned short code, unsigned short block);
    };
}

#endif