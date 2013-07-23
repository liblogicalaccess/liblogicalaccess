/**
 * \file tagitcommands.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Tag-It commands.
 */

#ifndef LOGICALACCESS_TAGITCOMMANDS_HPP
#define LOGICALACCESS_TAGITCOMMANDS_HPP

#include "logicalaccess/key.hpp"
#include "../iso15693/iso15693commands.hpp"

#include <string>
#include <vector>
#include <iostream>

namespace logicalaccess
{
	/**
	 * \brief The Tag-It commands base class.
	 */
	class LIBLOGICALACCESS_API TagItCommands : public ISO15693Commands
	{
		public:

			/**
			 * \brief Get if a block is locked.
			 * \param block The block number.
			 * \return True if the block is locked, false otherwise.
			 */
			virtual bool isLocked(size_t block);

		protected:

	};
}

#endif
