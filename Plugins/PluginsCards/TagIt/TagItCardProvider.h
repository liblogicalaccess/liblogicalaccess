/**
 * \file TagItCardProvider.h
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Tag-It card provider.
 */

#ifndef LOGICALACCESS_TAGITCARDPROVIDER_H
#define LOGICALACCESS_TAGITCARDPROVIDER_H

#include "logicalaccess/Key.h"
#include "../ISO15693/ISO15693CardProvider.h"

#include <string>
#include <vector>
#include <iostream>

using std::string;
using std::vector;
using std::ostream;

namespace logicalaccess
{
	/**
	 * \brief The Tag-It card provider base class.
	 */
	class LIBLOGICALACCESS_API TagItCardProvider : public ISO15693CardProvider
	{
		public:			

			/**
			 * \brief Constructor.
			 */
			TagItCardProvider();

			/**
			 * \brief Destructor.
			 */
			virtual ~TagItCardProvider();			

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
