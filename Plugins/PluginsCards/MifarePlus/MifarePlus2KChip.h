/**
 * \file MifarePlus2KChip.h
 * \author Xavier S. <xavier.schmerber@gmail.com>
 * \brief MifarePlus 2k chip.
 */

#ifndef LOGICALACCESS_MIFAREPLUS2KCHIP_H
#define LOGICALACCESS_MIFAREPLUS2KCHIP_H

#include "MifarePlusChip.h"

namespace LOGICALACCESS
{
	/**
	 * \brief The 2k MifarePlus base chip class.
	 */
	class LIBLOGICALACCESS_API MifarePlus2KChip : public MifarePlusChip
	{
		public:

			/**
			 * \brief Constructor.
			 */
			MifarePlus2KChip();

			/**
			 * \brief Destructor.
			 */
			virtual ~MifarePlus2KChip();

		protected:

	};
}

#endif /* LOGICALACCESS_MIFAREPLUS2KCHIP_H */
