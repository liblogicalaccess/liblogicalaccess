/**
 * \file mifareplus2kchip.hpp
 * \author Xavier S. <xavier.schmerber@gmail.com>
 * \brief MifarePlus 2k chip.
 */

#ifndef LOGICALACCESS_MIFAREPLUS2KCHIP_HPP
#define LOGICALACCESS_MIFAREPLUS2KCHIP_HPP

#include "mifarepluschip.hpp"

namespace logicalaccess
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

#endif /* LOGICALACCESS_MIFAREPLUS2KCHIP_HPP */
