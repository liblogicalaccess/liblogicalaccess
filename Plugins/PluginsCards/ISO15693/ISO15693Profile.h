/**
 * \file ISO15693Profile.h
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief ISO15693 card profiles.
 */

#ifndef LOGICALACCESS_ISO15693PROFILE_H
#define LOGICALACCESS_ISO15693PROFILE_H

#include "logicalaccess/Cards/AccessInfo.h"
#include "ISO15693Location.h"
#include "logicalaccess/Cards/Profile.h"

#include <string>
#include <vector>
#include <iostream>

using std::string;
using std::vector;
using std::ostream;

namespace logicalaccess
{
	/**
	 * \brief The ISO15693 base profile class.
	 */
	class LIBLOGICALACCESS_API ISO15693Profile : public Profile
	{
		public:			

			/**
			 * \brief Constructor.
			 */
			ISO15693Profile();

			/**
			 * \brief Destructor.
			 */
			virtual ~ISO15693Profile();

			/**
			 * \brief Set default keys for the card in memory.
			 */
			virtual void setDefaultKeys();

			/**
			 * \brief Set default keys for the type card in memory at a specific location.
			 */
			virtual void setDefaultKeysAt(boost::shared_ptr<Location> location);

			/**
			 * \brief Clear all keys in memory.
			 */
			virtual void clearKeys();

			/**
			 * \brief Set ISO15693 key at a specific location.
			 * \param location The key's location.
			 * \param AccessInfo The key's informations.
			 */
			virtual void setKeyAt(boost::shared_ptr<Location> location, boost::shared_ptr<AccessInfo> AccessInfo);

			/**
			 * \brief Set default keys for a sector.
			 * \param index The sector index.
			 */
			void setDefaultKeysAt(size_t index);
			
			/**
			 * \brief Create default ISO15693 access informations.
			 * \return Default ISO15693 access informations. Always null.
			 */
			virtual boost::shared_ptr<AccessInfo> createAccessInfo() const;

			/**
			 * \brief Create default ISO15693 location.
			 * \return Default ISO15693 location.
			 */
			virtual boost::shared_ptr<Location> createLocation() const;
			
			/**
			 * \brief Get the supported format list.
			 * \return The format list.
			 */
			virtual FormatList getSupportedFormatList();

		protected:

	};
}

#endif
