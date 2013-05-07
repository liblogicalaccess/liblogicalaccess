/**
 * \file DESFireEV1Chip.h
 * \author Maxime CHAMLEY <maxime.chamley@islog.eu>
 * \brief DESFire EV1 chip.
 */

#ifndef LOGICALACCESS_DESFIREEV1CHIP_H
#define LOGICALACCESS_DESFIREEV1CHIP_H

#include "DESFireChip.h"
#include "DESFireEV1CardProvider.h"
#include "DESFireEV1Profile.h"


namespace LOGICALACCESS
{	
	#define	CHIP_DESFIRE_EV1		"DESFireEV1"

	/**
	 * \brief The DESFire EV1 base chip class.
	 */
	class LIBLOGICALACCESS_API DESFireEV1Chip : public DESFireChip
	{
		public:

			/**
			 * \brief Constructor.
			 */
			DESFireEV1Chip();

			/**
			 * \brief Destructor.
			 */
			virtual ~DESFireEV1Chip();		

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
			boost::shared_ptr<DESFireEV1CardProvider> getDESFireEV1CardProvider() { return boost::dynamic_pointer_cast<DESFireEV1CardProvider>(getCardProvider()); };

			/**
			 * \brief Get the DESFire EV1 profile.
			 * \return The DESFire EV1 profile.
			 */
			boost::shared_ptr<DESFireEV1Profile> getDESFireEV1Profile() { return boost::dynamic_pointer_cast<DESFireEV1Profile>(getProfile()); };

		protected:
						
	};
}

#endif /* LOGICALACCESS_DESFIREEV1CHIP_H */

