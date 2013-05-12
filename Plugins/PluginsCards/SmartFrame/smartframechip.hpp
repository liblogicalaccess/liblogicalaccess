/**
 * \file smartframechip.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Smart Frame chip.
 */

#ifndef LOGICALACCESS_SMARTFRAMECHIP_HPP
#define LOGICALACCESS_SMARTFRAMECHIP_HPP

#include "logicalaccess/cards/chip.hpp"

#include <string>
#include <vector>
#include <iostream>

namespace logicalaccess
{
	#define CHIP_SMARTFRAME		"SmartFrame"

	/**
	 * \brief The Smart Frame base chip class.
	 */
	class LIBLOGICALACCESS_API SmartFrameChip : public Chip
	{
		public:			

			/**
			 * \brief Constructor.
			 */
			SmartFrameChip();

			/**
			 * \brief Destructor.
			 */
			virtual ~SmartFrameChip();

			/**
			 * \brief Get the generic card type.
			 * \return The generic card type.
			 */
			virtual std::string getGenericCardType() const { return CHIP_SMARTFRAME; };			

			/**
			 * \brief Get the root location node.
			 * \return The root location node.
			 */
			virtual boost::shared_ptr<LocationNode> getRootLocationNode();

		protected:

	};
}

#endif
