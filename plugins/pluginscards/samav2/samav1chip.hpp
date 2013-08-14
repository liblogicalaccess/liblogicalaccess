/**
 * \file desfireev1chip.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief DESFire EV1 chip.
 */

#ifndef LOGICALACCESS_SAMAV1CHIP_HPP
#define LOGICALACCESS_SAMAV1CHIP_HPP

#include "logicalaccess/cards/chip.hpp"
#include "samcommands.hpp"
#include "samchip.hpp"


namespace logicalaccess
{	
	#define CHIP_SAMAV1	"SAM_AV1"

	/**
	 * \brief The SAM chip class.
	 */
	class  SAMAV1Chip : public SAMChip
	{
		public:

			/**
			 * \brief Constructor.
			 */
			SAMAV1Chip();

			SAMAV1Chip::SAMAV1Chip(std::string ct);

			/**
			 * \brief Destructor.
			 */
			~SAMAV1Chip();


			boost::shared_ptr<SAMCommands> getSAMAV1Commands() { return boost::dynamic_pointer_cast<SAMCommands>(getCommands()); };
		protected:
	};
}

#endif /* LOGICALACCESS_DESFIREEV1CHIP_HPP */

