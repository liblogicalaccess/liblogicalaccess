/**
 * \file proxchip.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Prox chip.
 */

#ifndef LOGICALACCESS_PROXCHIP_HPP
#define LOGICALACCESS_PROXCHIP_HPP

#include "logicalaccess/cards/chip.hpp"
#include "proxprofile.hpp"

#include <string>
#include <vector>
#include <iostream>

namespace logicalaccess
{
	#define CHIP_PROX		"Prox"

	/**
	 * \brief The HID Prox base chip class.
	 */
	class LIBLOGICALACCESS_API ProxChip : public Chip
	{
		public:			

			/**
			 * \brief Constructor.
			 */
			ProxChip();

			/**
			 * \brief Destructor.
			 */
			virtual ~ProxChip();

			/**
			 * \brief Get the generic card type.
			 * \return The generic card type.
			 */
			virtual std::string getGenericCardType() const { return CHIP_PROX; };

			/**
			 * \brief Get the root location node.
			 * \return The root location node.
			 */
			virtual boost::shared_ptr<LocationNode> getRootLocationNode();

			/**
			 * \brief Get a card service for this card provider.
			 * \param serviceType The card service type.
			 * \return The card service.
			 */
			virtual boost::shared_ptr<CardService> getService(CardServiceType serviceType);

			/**
			 * \brief Get the Prox profile.
			 * \return The Prox profile.
			 */
			boost::shared_ptr<ProxProfile> getProxProfile() { return boost::dynamic_pointer_cast<ProxProfile>(getProfile()); };

		protected:

	};
}

#endif
