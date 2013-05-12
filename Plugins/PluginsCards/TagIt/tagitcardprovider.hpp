/**
 * \file tagitcardprovider.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Tag-It card provider.
 */

#ifndef LOGICALACCESS_TAGITCARDPROVIDER_HPP
#define LOGICALACCESS_TAGITCARDPROVIDER_HPP

#include "logicalaccess/key.hpp"
#include "../iso15693/iso15693cardprovider.hpp"

#include <string>
#include <vector>
#include <iostream>

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
