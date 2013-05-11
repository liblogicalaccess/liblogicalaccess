/**
 * \file MifareChip.h
 * \author Xavier S. <xavier.schmerber@gmail.com>
 * \brief MifarePlus chip.
 */

#ifndef LOGICALACCESS_MIFAREPLUSCHIP_H
#define LOGICALACCESS_MIFAREPLUSCHIP_H

#include "logicalaccess/Cards/Chip.h"
#include "MifarePlusCardProviderSL1.h"
#include "MifarePlusProfileSL1.h"

#include <string>
#include <vector>
#include <iostream>

using std::string;
using std::vector;
using std::ostream;

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

			/**
			 * \brief Get the Mifare card provider for I/O access.
			 * \return The Mifare card provider.
			 */
			boost::shared_ptr<MifarePlusCardProviderSL1> getMifarePlusCardProviderSL1() { return boost::dynamic_pointer_cast<MifarePlusCardProviderSL1>(getCardProvider()); };
/*
			boost::shared_ptr<MifarePlusCardProviderSL2> getMifarePlusCardProviderSL2() { return boost::dynamic_pointer_cast<MifarePlusCardProviderSL2>(getCardProvider()); };
			boost::shared_ptr<MifarePlusCardProviderSL3> getMifarePlusCardProviderSL3() { return boost::dynamic_pointer_cast<MifarePlusCardProviderSL3>(getCardProvider()); };
			boost::shared_ptr<MifarePlusCardProviderSL0> getMifarePlusCardProviderSL0() { return boost::dynamic_pointer_cast<MifarePlusCardProviderSL0>(getCardProvider()); };
			*/
			/**
			 * \brief Get the Mifare profile.
			 * \return The Mifare profile.
			 */
			inline boost::shared_ptr<MifarePlusProfileSL1> getMifarePlusProfileSL1() { return boost::dynamic_pointer_cast<MifarePlusProfileSL1>(getProfile()); };
/*			inline boost::shared_ptr<MifarePlusProfileSL2> getMifarePlusProfileSL2() { return boost::dynamic_pointer_cast<MifarePlusProfileSL2>(getProfile()); };
			inline boost::shared_ptr<MifarePlusProfileSL3> getMifarePlusProfileSL3() { return boost::dynamic_pointer_cast<MifarePlusProfileSL3>(getProfile()); };
			inline boost::shared_ptr<MifarePlusProfileSL0> getMifarePlusProfileSL0() { return boost::dynamic_pointer_cast<MifarePlusProfileSL0>(getProfile()); };*/

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

#endif /* LOGICALACCESS_MIFAREPLUSCHIP_H */
