/**
 * \file StmLri512Chip.h
 * \author Maxime CHAMLEY <maxime.chamley@islog.eu>
 * \brief STM LRI512 chip.
 */

#ifndef LOGICALACCESS_STMLRI512CHIP_H
#define LOGICALACCESS_STMLRI512CHIP_H

#include "logicalaccess/Cards/Chip.h"

#include <string>
#include <vector>
#include <iostream>

using std::string;
using std::vector;
using std::ostream;

namespace LOGICALACCESS
{
	#define CHIP_STMLRI512	"StmLri512"

	/**
	 * \brief The STM LRI512 base chip class.
	 */
	class LIBLOGICALACCESS_API StmLri512Chip : public Chip
	{
		public:			

			/**
			 * \brief Constructor.
			 */
			StmLri512Chip();

			/**
			 * \brief Destructor.
			 */
			virtual ~StmLri512Chip();

			/**
			 * \brief Get the generic card type.
			 * \return The generic card type.
			 */
			virtual std::string getGenericCardType() const { return CHIP_STMLRI512; };			

			/**
			 * \brief Get the root location node.
			 * \return The root location node.
			 */
			virtual boost::shared_ptr<LocationNode> getRootLocationNode();

		protected:

	};
}

#endif
