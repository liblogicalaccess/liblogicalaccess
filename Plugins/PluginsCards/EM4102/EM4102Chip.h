/**
 * \file EM4102Chip.h
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief EM4102 chip.
 */

#ifndef LOGICALACCESS_EM4102CHIP_H
#define LOGICALACCESS_EM4102CHIP_H

#include "logicalaccess/Cards/Chip.h"

#include <string>
#include <vector>
#include <iostream>

using std::string;
using std::vector;
using std::ostream;

namespace logicalaccess
{
	#define	CHIP_EM4102			"EM4102"
	/**
	 * \brief The EM4102 base chip class.
	 */
	class LIBLOGICALACCESS_API EM4102Chip : public Chip
	{
		public:			

			/**
			 * \brief Constructor.
			 */
			EM4102Chip();

			/**
			 * \brief Destructor.
			 */
			virtual ~EM4102Chip();

			/**
			 * \brief Get the generic card type.
			 * \return The generic card type.
			 */
			virtual std::string getGenericCardType() const { return CHIP_EM4102; };			

			/**
			 * \brief Get the root location node.
			 * \return The root location node.
			 */
			virtual boost::shared_ptr<LocationNode> getRootLocationNode();

		protected:

	};
}

#endif
