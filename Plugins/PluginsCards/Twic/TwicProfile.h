/**
 * \file TwicProfile.h
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Twic card profiles.
 */

#ifndef LOGICALACCESS_TWIC_H
#define LOGICALACCESS_TWIC_H

#include "logicalaccess/Cards/AccessInfo.h"
#include "../ISO7816/ISO7816Location.h"
#include "../ISO7816/ISO7816Profile.h"
#include "TwicCardProvider.h"

#include <string>
#include <vector>
#include <iostream>

using std::string;
using std::vector;
using std::ostream;

namespace logicalaccess
{
	/**
	 * \brief The Twic base profile class.
	 */
	class LIBLOGICALACCESS_API TwicProfile : public ISO7816Profile
	{
		public:			

			/**
			 * \brief Constructor.
			 */
			TwicProfile();

			/**
			 * \brief Destructor.
			 */
			virtual ~TwicProfile();

			/**
			 * \brief Create default Twic location.
			 * \return Default Twic location.
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
