/**
 * \file ICode2Chip.h
 * \author Maxime CHAMLEY <maxime.chamley@islog.eu>
 * \brief iCode2 chip.
 */

#ifndef LOGICALACCESS_ICODE2CHIP_H
#define LOGICALACCESS_ICODE2CHIP_H

#include "../ISO15693/ISO15693Chip.h"

#include <string>
#include <vector>
#include <iostream>

using std::string;
using std::vector;
using std::ostream;

namespace LOGICALACCESS
{
	/**
	 * \brief The iCode2 base chip class.
	 */
	class LIBLOGICALACCESS_API ICode2Chip : public ISO15693Chip
	{
		public:			

			/**
			 * \brief Constructor.
			 */
			ICode2Chip();

			/**
			 * \brief Destructor.
			 */
			virtual ~ICode2Chip();		

			/**
			 * \brief Get the root location node.
			 * \return The root location node.
			 */
			virtual boost::shared_ptr<LocationNode> getRootLocationNode();

		protected:

	};
}

#endif
