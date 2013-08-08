/**
 * \file desfireev1chip.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief DESFire EV1 chip.
 */

#ifndef LOGICALACCESS_SAMAV2CHIP_HPP
#define LOGICALACCESS_SAMAV2CHIP_HPP

#include "logicalaccess/cards/chip.hpp"
#include "samav2commands.hpp"
#include "samchip.hpp"


namespace logicalaccess
{	
	#define CHIP_SAMAV2	"SAM_AV2"

	/**
	 * \brief The SAMAV2 chip class.
	 */
	class  SAMAV2Chip : public SAMChip
	{
		public:

			/**
			 * \brief Constructor.
			 */
			SAMAV2Chip();

			SAMAV2Chip::SAMAV2Chip(std::string ct);

			/**
			 * \brief Destructor.
			 */
			~SAMAV2Chip();


			boost::shared_ptr<SAMAV2Commands> getSAMAV2Commands() { return boost::dynamic_pointer_cast<SAMAV2Commands>(getCommands()); };
		protected:
	};
}

#endif /* LOGICALACCESS_DESFIREEV1CHIP_HPP */

