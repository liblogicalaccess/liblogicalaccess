/**
 * \file SmartFrameChip.h
 * \author Maxime CHAMLEY <maxime.chamley@islog.eu>
 * \brief Smart Frame chip.
 */

#ifndef LOGICALACCESS_SMARTFRAMECHIP_H
#define LOGICALACCESS_SMARTFRAMECHIP_H

#include "logicalaccess/Cards/Chip.h"

#include <string>
#include <vector>
#include <iostream>

using std::string;
using std::vector;
using std::ostream;

namespace LOGICALACCESS
{
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
			virtual std::string getGenericCardType() const { return "SmartFrame"; };			

			/**
			 * \brief Get the root location node.
			 * \return The root location node.
			 */
			virtual boost::shared_ptr<LocationNode> getRootLocationNode();

		protected:

	};
}

#endif
