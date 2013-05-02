/**
 * \file MifarePlus4KChip.h
 * \author Xavier SCHMERBER <xavier.schmerber@gmail.com>
 * \brief MifarePlus 4k chip.
 */

#ifndef LOGICALACCESS_MIFAREPLUS4KCHIP_H
#define LOGICALACCESS_MIFAREPLUS4KCHIP_H

#include "MifarePlusChip.h"

namespace LOGICALACCESS
{
	/**
	 * \brief The 4k MifarePlus base chip class.
	 */
	class LIBLOGICALACCESS_API MifarePlus4KChip : public MifarePlusChip
	{
		public:

			/**
			 * \brief Constructor
			 */
			MifarePlus4KChip();

			/**
			 * \brief Destructor.
			 */
			virtual ~MifarePlus4KChip();

			/**
			 * \brief Get the root location node.
			 * \return The root location node.
			 */
			virtual boost::shared_ptr<LocationNode> getRootLocationNode();

		protected:

	};
}

#endif /* LOGICALACCESS_MIFARE4KCHIP_H */
