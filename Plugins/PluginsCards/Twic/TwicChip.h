/**
 * \file TwicChip.h
 * \author Maxime CHAMLEY <maxime.chamley@islog.eu>
 * \brief Twic chip.
 */

#ifndef LOGICALACCESS_TWICCHIP_H
#define LOGICALACCESS_TWICCHIP_H

#include "../ISO7816/ISO7816Chip.h"
#include "TwicCardProvider.h"
#include "TwicProfile.h"

#include <string>
#include <vector>
#include <iostream>

using std::string;
using std::vector;
using std::ostream;

namespace LOGICALACCESS
{
	#define CHIP_TWIC		"Twic"

	/**
	 * \brief The Twic base chip class.
	 */
	class LIBLOGICALACCESS_API TwicChip : public ISO7816Chip
	{
		public:			

			/**
			 * \brief Constructor.
			 */
			TwicChip();

			/**
			 * \brief Destructor.
			 */
			virtual ~TwicChip();				

			/**
			 * \brief Get the root location node.
			 * \return The root location node.
			 */
			virtual boost::shared_ptr<LocationNode> getRootLocationNode();

			/**
			 * \brief Get the Twic card provider for I/O access.
			 * \return The Twic card provider.
			 */
			boost::shared_ptr<TwicCardProvider> getTwicCardProvider() { return boost::dynamic_pointer_cast<TwicCardProvider>(getISO7816CardProvider()); };

			/**
			 * \brief Get the Twic profile.
			 * \return The Twic profile.
			 */
			boost::shared_ptr<TwicProfile> getTwicProfile() { return boost::dynamic_pointer_cast<TwicProfile>(getProfile()); };

		protected:

	};
}

#endif
