/**
 * \file DESFireChip.h
 * \author Maxime CHAMLEY <maxime.chamley@islog.eu>
 * \brief DESFire chip.
 */

#ifndef LOGICALACCESS_DESFIRECHIP_H
#define LOGICALACCESS_DESFIRECHIP_H

#include "logicalaccess/Cards/Chip.h"
#include "DESFireCardProvider.h"
#include "DESFireProfile.h"

#include <string>
#include <vector>
#include <iostream>

using std::string;
using std::vector;
using std::ostream;


namespace LOGICALACCESS
{	
	#define	CHIP_DESFIRE		"DESFire"

	/**
	 * \brief The DESFire base chip class.
	 */
	class LIBLOGICALACCESS_API DESFireChip : public Chip
	{
		public:

			/**
			 * \brief Constructor.
			 */
			DESFireChip();

			/**
			 * \brief Constructor.
			 * \param ct The card type.
			 */
			DESFireChip(std::string ct);

			/**
			 * \brief Destructor.
			 */
			virtual ~DESFireChip();

			/**
			 * \brief Get the generic card type.
			 * \return The generic card type.
			 */
			virtual std::string getGenericCardType() const { return CHIP_DESFIRE; };				

			/**
			 * \brief Get the root location node.
			 * \return The root location node.
			 */
			virtual boost::shared_ptr<LocationNode> getRootLocationNode();

			/**
			 * \brief Get the application location information.
			 * \return The location.
			 */
			virtual boost::shared_ptr<DESFireLocation> getApplicationLocation();

			/**
			 * \brief Get the DESFire card provider for I/O access.
			 * \return The DESFire card provider.
			 */
			boost::shared_ptr<DESFireCardProvider> getDESFireCardProvider() { return boost::dynamic_pointer_cast<DESFireCardProvider>(getCardProvider()); };

			/**
			 * \brief Get the DESFire profile.
			 * \return The DESFire profile.
			 */
			boost::shared_ptr<DESFireProfile> getDESFireProfile() { return boost::dynamic_pointer_cast<DESFireProfile>(getProfile()); };

		protected:
						
	};
}

#endif /* LOGICALACCESS_DESFIRECHIP_H */

