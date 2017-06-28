/**
 * \file mifarechip.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Mifare chip.
 */

#ifndef LOGICALACCESS_MIFARECHIP_HPP
#define LOGICALACCESS_MIFARECHIP_HPP

#include "logicalaccess/cards/chip.hpp"
#include "mifarecommands.hpp"

#include <string>
#include <vector>
#include <iostream>

namespace logicalaccess
{
#define CHIP_MIFARE 	"Mifare"

    /**
     * \brief The Mifare base chip class.
     */
    class LIBLOGICALACCESS_API MifareChip : public
// SWIG ne semble pas gerer l'heritage virtuel, il l'ignore totalement.
#ifndef SWIG
		virtual 
#endif		
		Chip
    {
    public:

        /**
         * \brief Constructor for generic Mifare.
         */
        MifareChip();

        /**
         * \brief Destructor.
         */
        virtual ~MifareChip();

        /**
         * \brief Get the generic card type.
         * \return The generic card type.
         */
        virtual std::string getGenericCardType() const { return CHIP_MIFARE; };

        /**
         * \brief Get the root location node.
         * \return The root location node.
         */
        virtual std::shared_ptr<LocationNode> getRootLocationNode();

        /**
         * \brief Get number of sectors
         */
        unsigned int getNbSectors() const;

        /**
         * \brief Add a sector node to a root node.
         * \param rootNode The root node.
         * \param sector The sector.
         */
        void addSectorNode(std::shared_ptr<LocationNode> rootNode, int sector);

		/**
		* \brief Add a block node to a root node.
		* \param rootNode The root node.
		* \param sector The sector.
		* \param block The block.
		*/
		void addBlockNode(std::shared_ptr<LocationNode> rootNode, int sector, unsigned char block);

        /**
         * \brief Get a card service for this card provider.
         * \param serviceType The card service type.
         * \return The card service.
         */
        virtual std::shared_ptr<CardService> getService(CardServiceType serviceType);

		/**
		* \brief Create default Mifare access informations.
		* \return Default Mifare access informations.
		*/
		virtual std::shared_ptr<AccessInfo> createAccessInfo() const;

		/**
		* \brief Create default Mifare location.
		* \return Default Mifare location.
		*/
		virtual std::shared_ptr<Location> createLocation() const;

        /**
         * \brief Get the Mifare commands.
         * \return The Mifare commands.
         */
        std::shared_ptr<MifareCommands> getMifareCommands() { return std::dynamic_pointer_cast<MifareCommands>(getCommands()); };

    protected:

        /**
         * \brief Constructor.
         * \param cardtype The card type (Mifare 1K, Mifare 4K, ...).
         * \param nbSectors The number of sectors in the card.
         */
        MifareChip(std::string cardtype, unsigned int nbSectors);

        /**
         * \brief The number of sectors in the Mifare card.
         */
        unsigned int d_nbSectors;
    };
}

#endif /* LOGICALACCESS_MIFARECHIP_HPP */