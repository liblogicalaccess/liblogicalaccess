/**
 * \file mifare1kchip.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Mifare 1k chip.
 */

#ifndef LOGICALACCESS_MIFARE1KCHIP_HPP
#define LOGICALACCESS_MIFARE1KCHIP_HPP

#include "mifarechip.hpp"

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

#endif /* LOGICALACCESS_MIFARE1KCHIP_HPP */

