/**
 * \file Mifare4KChip.h
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Mifare 4k chip.
 */

#ifndef LOGICALACCESS_MIFARE4KCHIP_H
#define LOGICALACCESS_MIFARE4KCHIP_H

#include "MifareChip.h"

namespace LOGICALACCESS
{
	#define CHIP_MIFARE4K	"Mifare4K"

	/**
	 * \brief The 4k Mifare base chip class.
	 */
	class LIBLOGICALACCESS_API Mifare4KChip : public MifareChip
	{
		public:

			/**
			 * \brief Constructor
			 */
			Mifare4KChip();

			/**
			 * \brief Destructor.
			 */
			virtual ~Mifare4KChip();

			/**
			 * \brief Get the root location node.
			 * \return The root location node.
			 */
			virtual boost::shared_ptr<LocationNode> getRootLocationNode();

		protected:

	};
}

#endif /* LOGICALACCESS_MIFARE4KCHIP_H */

