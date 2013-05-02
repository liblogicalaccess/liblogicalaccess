/**
 * \file LegicPrimeChip.h
 * \author Maxime CHAMLEY <maxime.chamley@islog.eu>
 * \brief Legic Prime chip.
 */

#ifndef LOGICALACCESS_LEGICPRIMECHIP_H
#define LOGICALACCESS_LEGICPRIMECHIP_H

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
	 * \brief The Legic Prime base chip class.
	 */
	class LIBLOGICALACCESS_API LegicPrimeChip : public Chip
	{
		public:			

			/**
			 * \brief Constructor.
			 */
			LegicPrimeChip();

			/**
			 * \brief Destructor.
			 */
			virtual ~LegicPrimeChip();

			/**
			 * \brief Get the generic card type.
			 * \return The generic card type.
			 */
			virtual std::string getGenericCardType() const { return "LegicPrime"; };			

			/**
			 * \brief Get the root location node.
			 * \return The root location node.
			 */
			virtual boost::shared_ptr<LocationNode> getRootLocationNode();

		protected:

	};
}

#endif
