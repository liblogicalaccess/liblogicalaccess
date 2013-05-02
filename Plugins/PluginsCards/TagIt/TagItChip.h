/**
 * \file TagItChip.h
 * \author Maxime CHAMLEY <maxime.chamley@islog.eu>
 * \brief Tag-It chip.
 */

#ifndef LOGICALACCESS_TAGITCHIP_H
#define LOGICALACCESS_TAGITCHIP_H

#include "../ISO15693/ISO15693Chip.h"
#include "TagItCardProvider.h"

#include <string>
#include <vector>
#include <iostream>

using std::string;
using std::vector;
using std::ostream;

namespace LOGICALACCESS
{
	/**
	 * \brief The Tag-It base chip class.
	 */
	class LIBLOGICALACCESS_API TagItChip: public ISO15693Chip
	{
		public:			

			/**
			 * \brief Constructor.
			 */
			TagItChip();

			/**
			 * \brief Destructor.
			 */
			virtual ~TagItChip();

			/**
			 * \brief Get the root location node.
			 * \return The root location node.
			 */
			virtual boost::shared_ptr<LocationNode> getRootLocationNode();

			/**
			 * \brief Get the Tag-It card provider for I/O access.
			 * \return The Tag-It card provider.
			 */
			boost::shared_ptr<TagItCardProvider> getTagItCardProvider() { return boost::dynamic_pointer_cast<TagItCardProvider>(getISO15693CardProvider()); };

		protected:

	};
}

#endif
