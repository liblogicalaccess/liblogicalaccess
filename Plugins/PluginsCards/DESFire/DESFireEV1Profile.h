/**
 * \file DESFireEV1Profile.h
 * \author Maxime CHAMLEY <maxime.chamley@islog.eu>
 * \brief DESFire EV1 card profiles.
 */

#ifndef LOGICALACCESS_DESFIREEV1_H
#define LOGICALACCESS_DESFIREEV1_H

#include "DESFireProfile.h"
#include "DESFireEV1Location.h"


namespace LOGICALACCESS
{	
	/**
	 * \brief The DESFire EV1 base profile class.
	 */
	class LIBLOGICALACCESS_API DESFireEV1Profile : public DESFireProfile
	{
		public:

			/**
			 * \brief Constructor.
			 */
			DESFireEV1Profile();

			/**
			 * \brief Destructor.
			 */
			virtual ~DESFireEV1Profile();	

			/**
			 * \brief Set default keys for the type card in memory at a specific location.
			 */
			virtual void setDefaultKeysAt(boost::shared_ptr<Location> location);

			/**
			 * \brief Create default DESFire location.
			 * \return Default DESFire location.
			 */
			virtual boost::shared_ptr<Location> createLocation() const;
	};
}

#endif /* LOGICALACCESS_DESFIREEV1_H */

