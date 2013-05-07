/**
 * \file ProxLiteChip.h
 * \author Maxime CHAMLEY <maxime.chamley@islog.eu>
 * \brief Prox Lite chip.
 */

#ifndef LOGICALACCESS_PROXLITECHIP_H
#define LOGICALACCESS_PROXLITECHIP_H

#include "logicalaccess/Cards/Chip.h"

#include <string>
#include <vector>
#include <iostream>

using std::string;
using std::vector;
using std::ostream;

namespace LOGICALACCESS
{
	#define CHIP_PROXLITE	"ProxLite"

	/**
	 * \brief The Prox Lite base chip class.
	 */
	class LIBLOGICALACCESS_API ProxLiteChip : public Chip
	{
		public:			

			/**
			 * \brief Constructor.
			 */
			ProxLiteChip();

			/**
			 * \brief Destructor.
			 */
			virtual ~ProxLiteChip();

			/**
			 * \brief Get the generic card type.
			 * \return The generic card type.
			 */
			virtual std::string getGenericCardType() const { return CHIP_PROXLITE; };			

			/**
			 * \brief Get the root location node.
			 * \return The root location node.
			 */
			virtual boost::shared_ptr<LocationNode> getRootLocationNode();

		protected:

	};
}

#endif
