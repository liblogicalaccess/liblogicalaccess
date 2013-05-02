/**
 * \file Profile.h
 * \author Julien KAUFFMANN <julien.kauffmann@islog.eu>, Maxime CHAMLEY <maxime.chamley@islog.eu>
 * \brief Card profiles.
 */

#ifndef LOGICALACCESS_PROFILES_H
#define LOGICALACCESS_PROFILES_H

#include "logicalaccess/ReaderProviders/ReaderUnit.h"
#include "logicalaccess/Services/AccessControl/Formats/Format.h"
#include "logicalaccess/Cards/Location.h"

#include "logicalaccess/logs.h"

#ifdef LINUX
#include <wintypes.h>
#endif

namespace LOGICALACCESS
{
	class AccessInfo;	

	/**
	 * \brief A format list.
	 */
	typedef vector<boost::shared_ptr<Format> > FormatList;

	/**
	 * \brief The base profile class for all profiles. Most of the time, a profile only contains key mapping for a chip.
	 */
	class LIBLOGICALACCESS_API Profile : public boost::enable_shared_from_this<Profile>
	{
		public:

			/**
			 * \brief Constructor.
			 */
			Profile();

			/**
			 * \brief Destructor.
			 */
			virtual ~Profile() {};				

			/**
			 * \brief Set default keys for the type card in memory.
			 */
			virtual void setDefaultKeys() = 0;

			/**
			 * \brief Set default keys for the type card in memory at a specific location.
			 */
			virtual void setDefaultKeysAt(boost::shared_ptr<Location> location) = 0;

			/**
			 * \brief Set key at a specific location.
			 * \param location The key's location.
			 * \param AccessInfo The key's informations.
			 */
			virtual void setKeyAt(boost::shared_ptr<Location> location, boost::shared_ptr<AccessInfo> AccessInfo) = 0;

			/**
			 * \brief Clear all keys in memory
			 */
			virtual void clearKeys() = 0;

			/**
			 * \brief Create default access informations.
			 * \return Default access informations.
			 */
			virtual boost::shared_ptr<AccessInfo> createAccessInfo() const = 0;

			/**
			 * \brief Create default location.
			 * \return Default location.
			 */
			virtual boost::shared_ptr<Location> createLocation() const = 0;

			/**
			 * \brief Get the supported format list.
			 * \return The format list.
			 */
			virtual FormatList getSupportedFormatList() = 0;

			/**
			 * \brief Get the HID Wiegand format list.
			 * \return The format list.
			 */
			FormatList getHIDWiegandFormatList();

			/**
			 * \brief Get the profile name, if specified.
			 * \return The profile name.
			 */
			virtual std::string getName() const { return ""; };

		protected:

	};
}

#endif /* LOGICALACCESS_PROFILES_H */

