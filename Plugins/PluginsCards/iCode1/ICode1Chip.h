/**
 * \file ICode1Chip.h
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief iCode1 chip.
 */

#ifndef LOGICALACCESS_ICODE1CHIP_H
#define LOGICALACCESS_ICODE1CHIP_H

#include "../ISO15693/ISO15693Chip.h"

#include <string>
#include <vector>
#include <iostream>

using std::string;
using std::vector;
using std::ostream;

namespace logicalaccess
{
	#define CHIP_ICODE1		"iCode1"

	/**
	 * \brief The iCode1 base chip class.
	 */
	class LIBLOGICALACCESS_API ICode1Chip : public ISO15693Chip
	{
		public:			

			/**
			 * \brief Constructor.
			 */
			ICode1Chip();

			/**
			 * \brief Destructor.
			 */
			virtual ~ICode1Chip();		

			/**
			 * \brief Get the root location node.
			 * \return The root location node.
			 */
			virtual boost::shared_ptr<LocationNode> getRootLocationNode();

		protected:

	};
}

#endif
