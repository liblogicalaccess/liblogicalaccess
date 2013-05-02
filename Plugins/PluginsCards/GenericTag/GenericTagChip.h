/**
 * \file GenericTagChip.h
 * \author Maxime CHAMLEY <maxime.chamley@islog.eu>
 * \brief Generic tag chip.
 */

#ifndef LOGICALACCESS_GENERICTAGCHIP_H
#define LOGICALACCESS_GENERICTAGCHIP_H

#include "logicalaccess/Cards/Chip.h"
#include "GenericTagCardProvider.h"

#include <string>
#include <vector>
#include <iostream>

using std::string;
using std::vector;
using std::ostream;

namespace LOGICALACCESS
{
	/**
	 * \brief The Generic Tag base chip class.
	 */
	class LIBLOGICALACCESS_API GenericTagChip : public Chip
	{
		public:			

			/**
			 * \brief Constructor.
			 */
			GenericTagChip();

			/**
			 * \brief Destructor.
			 */
			virtual ~GenericTagChip();

			/**
			 * \brief Get the generic card type.
			 * \return The generic card type.
			 */
			virtual std::string getGenericCardType() const { return "GenericTag"; };			

			/**
			 * \brief Get the root location node.
			 * \return The root location node.
			 */
			virtual boost::shared_ptr<LocationNode> getRootLocationNode();

			/**
			 * \brief Get the Generic Tag card provider for I/O access.
			 * \return The Generic Tag card provider.
			 */
			boost::shared_ptr<GenericTagCardProvider> getGenericTagCardProvider() { return boost::dynamic_pointer_cast<GenericTagCardProvider>(getCardProvider()); };

			void setTagIdBitsLength(unsigned int length) { d_tagIdBitsLength = length; };

			unsigned int getTagIdBitsLength() const { return d_tagIdBitsLength; };

		protected:

			/**
			 * \brief The tag id length in bits (if 0, the length is the full bytes).
			 */
			unsigned int d_tagIdBitsLength;
	};
}

#endif
