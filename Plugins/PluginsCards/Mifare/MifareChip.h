/**
 * \file MifareChip.h
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Mifare chip.
 */

#ifndef LOGICALACCESS_MIFARECHIP_H
#define LOGICALACCESS_MIFARECHIP_H

#include "logicalaccess/Cards/Chip.h"
#include "MifareCardProvider.h"
#include "MifareProfile.h"

#include <string>
#include <vector>
#include <iostream>

using std::string;
using std::vector;
using std::ostream;

namespace LOGICALACCESS
{	
	#define CHIP_MIFARE4K	"Mifare"

	/**
	 * \brief The Mifare base chip class.
	 */
	class LIBLOGICALACCESS_API MifareChip : public Chip
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
			virtual std::string getGenericCardType() const { return "Mifare"; };					

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
			boost::shared_ptr<MifareCardProvider> getMifareCardProvider() { return boost::dynamic_pointer_cast<MifareCardProvider>(getCardProvider()); };

			/**
			 * \brief Get the Mifare profile.
			 * \return The Mifare profile.
			 */
			boost::shared_ptr<MifareProfile> getMifareProfile() { return boost::dynamic_pointer_cast<MifareProfile>(getProfile()); };

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

#endif /* LOGICALACCESS_MIFARECHIP_H */

