/**
 * \file cardservice.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Card service.
 */

#ifndef LOGICALACCESS_CARDSERVICE_HPP
#define LOGICALACCESS_CARDSERVICE_HPP

#include "logicalaccess/readerproviders/readerprovider.hpp"

#include "logicalaccess/logs.hpp"

#ifdef LINUX
#include <wintypes.h>
#endif

namespace logicalaccess
{
	class CardProvider;

	/**
	 * \brief The card services.
	 */
	typedef enum {
		CST_ACCESS_CONTROL = 0x0000, /**< The access control card service type */
		CST_NFC_TAG = 0x0001 /**< The NFC Tag card service type */
	} CardServiceType;

	/**
	 * \brief The base card service class for all services.
	 */
	class LIBLOGICALACCESS_API CardService : public boost::enable_shared_from_this<CardService>
	{
		public:

			/**
			 * \brief Constructor.
			 * \param cardProvider The associated card provider.
			 */
			CardService(boost::shared_ptr<CardProvider> cardProvider);

			/*
			 * \brief Get the associated chip object.
			 * \return The chip.
			 */
			boost::shared_ptr<CardProvider> getCardProvider();

			/**
			 * \brief Get the card service type.
			 * \return The card service type.
			 */
			virtual CardServiceType getServiceType() const = 0;

		protected:

			/**
			 * \brief Card provider object.
			 */
			boost::shared_ptr<CardProvider> d_cardProvider;
	};
}

#endif