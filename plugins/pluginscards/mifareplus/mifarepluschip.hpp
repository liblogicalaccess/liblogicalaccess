/**
 * \file mifarechip.hpp
 * \author Xavier S. <xavier.schmerber@gmail.com>
 * \brief MifarePlus chip.
 */

#ifndef LOGICALACCESS_MIFAREPLUSCHIP_HPP
#define LOGICALACCESS_MIFAREPLUSCHIP_HPP

#include "logicalaccess/cards/chip.hpp"
#include "mifareplussl1commands.hpp"
#include "mifareplussl3commands.hpp"
#include "mifareplussl1profile.hpp"
#include "mifareplussl3profile.hpp"

#include <string>
#include <vector>
#include <iostream>

namespace logicalaccess
{
#define	CHIP_MIFAREPLUS4K			"MifarePlus4K"

    /**
     * \brief The Mifare base chip class.
     */
    class LIBLOGICALACCESS_API MifarePlusChip : public Chip
    {
    public:

        /**
         * \brief Constructor for generic Mifare.
         */
        MifarePlusChip();

        /**
         * \brief Destructor.
         */
        virtual ~MifarePlusChip();

        /**
         * \brief Get the generic card type.
         * \return The generic card type.
         */
        virtual std::string getGenericCardType() const { return CHIP_MIFAREPLUS4K; };

        /**
         * \brief Get the root location node.
         * \return The root location node.
         */
        virtual boost::shared_ptr<LocationNode> getRootLocationNode();

        /**
         * \brief Get number of sectors
         */
        unsigned int getNbSectors() const;

        /**
         * \brief Add a sector node the a root node.
         * \param rootNode The root node.
         * \param sector The sector.
         */
        void addSectorNode(boost::shared_ptr<LocationNode> rootNode, int sector);

        boost::shared_ptr<CardService> getService(CardServiceType serviceType);

        /**
         * \brief Get the Mifare card provider for I/O access.
         * \return The Mifare card provider.
         */
        boost::shared_ptr<MifarePlusSL1Commands> getMifarePlusSL1Commands() { return boost::dynamic_pointer_cast<MifarePlusSL1Commands>(getCommands()); };
        boost::shared_ptr<MifarePlusSL3Commands> getMifarePlusSL3Commands() { return boost::dynamic_pointer_cast<MifarePlusSL3Commands>(getCommands()); };

        /**
         * \brief Get the Mifare profile.
         * \return The Mifare profile.
         */
        inline boost::shared_ptr<MifarePlusSL1Profile> getMifarePlusSL1Profile() { return boost::dynamic_pointer_cast<MifarePlusSL1Profile>(getProfile()); };
        inline boost::shared_ptr<MifarePlusSL3Profile> getMifarePlusSL3Profile() { return boost::dynamic_pointer_cast<MifarePlusSL3Profile>(getProfile()); };

    protected:

        /**
         * \brief Constructor.
         * \param cardtype The card type (MifarePlus 2K, MifarePlus 4K, ...).
         * \param nbSectors The number of sectors in the card.
         */
        MifarePlusChip(std::string cardtype, unsigned int nbSectors);

        /**
         * \brief The number of sectors in the Mifare card.
         */
        unsigned int d_nbSectors;
    };
}

#endif /* LOGICALACCESS_MIFAREPLUSCHIP_HPP */