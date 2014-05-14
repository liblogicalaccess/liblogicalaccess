/**
 * \file generictagchip.h
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Generic tag chip.
 */

#ifndef LOGICALACCESS_GENERICTAGCHIP_HPP
#define LOGICALACCESS_GENERICTAGCHIP_HPP

#include "logicalaccess/cards/chip.hpp"

#include <string>
#include <vector>
#include <iostream>

namespace logicalaccess
{
	#define CHIP_GENERICTAG			"GenericTag"

	/**
	 * \brief The Generic Tag base chip class.
	 */
	class LIBLOGICALACCESS_API GenericTagChip : public Chip
	{
		public:	

            /**
			 * \brief Constructor.
             * \param cardtype The card type.
			 */
            GenericTagChip(std::string cardtype);

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
			virtual std::string getGenericCardType() const { return CHIP_GENERICTAG; };			

			/**
			 * \brief Get the root location node.
			 * \return The root location node.
			 */
			virtual boost::shared_ptr<LocationNode> getRootLocationNode();

			void setTagIdBitsLength(unsigned int length) { d_tagIdBitsLength = length; };

			unsigned int getTagIdBitsLength() const { return d_tagIdBitsLength; };

			/**
			 * \brief Get a card service for this card provider.
			 * \param serviceType The card service type.
			 * \return The card service.
			 */
			virtual boost::shared_ptr<CardService> getService(CardServiceType serviceType);

		protected:

			/**
			 * \brief The tag id length in bits (if 0, the length is the full bytes).
			 */
			unsigned int d_tagIdBitsLength;
	};
}

#endif
