/**
 * \file samav2chip.hpp
 * \author Adrien J. <adrien.jund@islog.com>
 * \brief SAMAV2Chip header.
 */

#ifndef LOGICALACCESS_SAMAV2CHIP_HPP
#define LOGICALACCESS_SAMAV2CHIP_HPP

#include "logicalaccess/cards/chip.hpp"
#include "samcommands.hpp"
#include "samchip.hpp"


namespace logicalaccess
{	
	#define CHIP_SAMAV2	"SAM_AV2"

	/**
	 * \brief The SAM chip class.
	 */
	class LIBLOGICALACCESS_API SAMAV2Chip : public SAMChip
	{
		public:

			/**
			 * \brief Constructor.
			 */
			SAMAV2Chip();

			SAMAV2Chip(std::string ct);

			/**
			 * \brief Destructor.
			 */
			~SAMAV2Chip();


			boost::shared_ptr<SAMCommands<KeyEntryAV2Information, SETAV2> > getSAMAV2Commands() { return boost::dynamic_pointer_cast<SAMCommands<KeyEntryAV2Information, SETAV2> >(getCommands()); };
		protected:
	};
}

#endif /* LOGICALACCESS_SAMAV2CHIP_HPP */

