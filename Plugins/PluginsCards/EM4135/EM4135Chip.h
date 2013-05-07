/**
 * \file EM4135Chip.h
 * \author Maxime CHAMLEY <maxime.chamley@islog.eu>
 * \brief EM4135 chip.
 */

#ifndef LOGICALACCESS_EM4135CHIP_H
#define LOGICALACCESS_EM4135CHIP_H

#include "logicalaccess/Cards/Chip.h"

#include <string>
#include <vector>
#include <iostream>

using std::string;
using std::vector;
using std::ostream;

namespace LOGICALACCESS
{
	#define CHIP_EM4135			"EM4135"
	/**
	 * \brief The EM4135 base chip class.
	 */
	class LIBLOGICALACCESS_API EM4135Chip : public Chip
	{
		public:			

			/**
			 * \brief Constructor.
			 */
			EM4135Chip();

			/**
			 * \brief Destructor.
			 */
			virtual ~EM4135Chip();

			/**
			 * \brief Get the generic card type.
			 * \return The generic card type.
			 */
			virtual std::string getGenericCardType() const { return CHIP_EM4135; };			

			/**
			 * \brief Get the root location node.
			 * \return The root location node.
			 */
			virtual boost::shared_ptr<LocationNode> getRootLocationNode();

		protected:

	};
}

#endif
