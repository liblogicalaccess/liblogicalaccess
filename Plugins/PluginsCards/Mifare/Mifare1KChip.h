/**
 * \file Mifare1KChip.h
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Mifare 1k chip.
 */

#ifndef LOGICALACCESS_MIFARE1KCHIP_H
#define LOGICALACCESS_MIFARE1KCHIP_H

#include "MifareChip.h"

namespace logicalaccess
{
	#define CHIP_MIFARE1K	"Mifare1K"

	/**
	 * \brief The 1k Mifare base chip class.
	 */
	class LIBLOGICALACCESS_API Mifare1KChip : public MifareChip
	{
		public:

			/**
			 * \brief Constructor.
			 */
			Mifare1KChip();

			/**
			 * \brief Destructor.
			 */
			virtual ~Mifare1KChip();

		protected:

	};
}

#endif /* LOGICALACCESS_MIFARE1KCHIP_H */

