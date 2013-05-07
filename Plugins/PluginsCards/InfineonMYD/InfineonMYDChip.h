/**
 * \file InfineonMYDChip.h
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Infineon My-D chip.
 */

#ifndef LOGICALACCESS_INFINEONMYDCHIP_H
#define LOGICALACCESS_INFINEONMYDCHIP_H

#include "../ISO15693/ISO15693Chip.h"

#include <string>
#include <vector>
#include <iostream>

using std::string;
using std::vector;
using std::ostream;

namespace LOGICALACCESS
{
	#define CHIP_INFINEONMYD	"InfineonMYD"

	/**
	 * \brief The Infineon My-D base chip class.
	 */
	class LIBLOGICALACCESS_API InfineonMYDChip : public ISO15693Chip
	{
		public:			

			/**
			 * \brief Constructor.
			 */
			InfineonMYDChip();

			/**
			 * \brief Destructor.
			 */
			virtual ~InfineonMYDChip();			

			/**
			 * \brief Get the root location node.
			 * \return The root location node.
			 */
			virtual boost::shared_ptr<LocationNode> getRootLocationNode();

		protected:

	};
}

#endif
