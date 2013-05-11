/**
 * \file MifareUltralightChip.h
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Mifare Ultralight chip.
 */

#ifndef LOGICALACCESS_MIFAREULTRALIGHTCCHIP_H
#define LOGICALACCESS_MIFAREULTRALIGHTCCHIP_H

#include "MifareUltralightChip.h"

#include "MifareUltralightCCardProvider.h"
#include "MifareUltralightCProfile.h"

#include <string>
#include <vector>
#include <iostream>

using std::string;
using std::vector;
using std::ostream;

namespace logicalaccess
{
	#define CHIP_MIFAREULTRALIGHTC		"MifareUltralightC"

	/**
	 * \brief The Mifare Ultralight C base chip class.
	 */
	class LIBLOGICALACCESS_API MifareUltralightCChip : public MifareUltralightChip
	{
		public:			

			/**
			 * \brief Constructor.
			 */
			MifareUltralightCChip();

			/**
			 * \brief Destructor.
			 */
			virtual ~MifareUltralightCChip();			

			/**
			 * \brief Get the root location node.
			 * \return The root location node.
			 */
			virtual boost::shared_ptr<LocationNode> getRootLocationNode();

			/**
			 * \brief Get the Mifare Ultralight C card provider for I/O access.
			 * \return The Mifare Ultralight C card provider.
			 */
			boost::shared_ptr<MifareUltralightCCardProvider> getMifareUltralightCCardProvider() { return boost::dynamic_pointer_cast<MifareUltralightCCardProvider>(getMifareUltralightCardProvider()); };

			/**
			 * \brief Get the Mifare Ultralight C profile.
			 * \return The Mifare Ultralight C profile.
			 */
			boost::shared_ptr<MifareUltralightCProfile> getMifareUltralightCProfile() { return boost::dynamic_pointer_cast<MifareUltralightCProfile>(getMifareUltralightCardProvider()); };

		protected:
	};
}

#endif
