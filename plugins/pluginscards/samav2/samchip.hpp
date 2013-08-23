/**
 * \file desfireev1chip.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief DESFire EV1 chip.
 */

#ifndef LOGICALACCESS_SAMCHIP_HPP
#define LOGICALACCESS_SAMCHIP_HPP

#include "logicalaccess/cards/chip.hpp"
#include "samcommands.hpp"


namespace logicalaccess
{	
	#define CHIP_SAM "SAM"
	/**
	 * \brief The SAM chip class.
	 */
	class LIBLOGICALACCESS_API SAMChip : public Chip
	{
		public:

			/**
			 * \brief Constructor.
			 */
			SAMChip::SAMChip() : Chip(CHIP_SAM) {};

			/**
			 * \brief Constructor.
			 */
			SAMChip::SAMChip(std::string t) : Chip(t) {};

			/**
			 * \brief Destructor.
			 */
			~SAMChip() {};

		protected:
	};
}

#endif /* LOGICALACCESS_DESFIREEV1CHIP_HPP */

