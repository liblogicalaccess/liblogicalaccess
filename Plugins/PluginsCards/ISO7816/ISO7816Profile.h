/**
 * \file ISO7816Profile.h
 * \author Maxime CHAMLEY <maxime.chamley@islog.eu>
 * \brief ISO7816 card profiles.
 */

#ifndef LOGICALACCESS_ISO7816PROFILE_H
#define LOGICALACCESS_ISO7816PROFILE_H

#include "logicalaccess/Cards/AccessInfo.h"
#include "ISO7816Location.h"
#include "logicalaccess/Cards/Profile.h"

#include <string>
#include <vector>
#include <iostream>

using std::string;
using std::vector;
using std::ostream;

namespace LOGICALACCESS
{
	/**
	 * \brief The ISO7816 base profile class.
	 */
	class LIBLOGICALACCESS_API ISO7816Profile : public Profile
	{
		public:			

			/**
			 * \brief Constructor.
			 */
			ISO7816Profile();

			/**
			 * \brief Destructor.
			 */
			virtual ~ISO7816Profile();

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
			 * \brief Set ISO7816 key at a specific location.
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
			 * \brief Create default ISO7816 access informations.
			 * \return Default ISO7816 access informations. Always null.
			 */
			virtual boost::shared_ptr<AccessInfo> createAccessInfo() const;

			/**
			 * \brief Create default ISO7816 location.
			 * \return Default ISO7816 location.
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
