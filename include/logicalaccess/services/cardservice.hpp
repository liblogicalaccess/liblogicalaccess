/**
 * \file cardservice.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Card service.
 */

#ifndef LOGICALACCESS_CARDSERVICE_HPP
#define LOGICALACCESS_CARDSERVICE_HPP

#include "logicalaccess/readerproviders/readerprovider.hpp"

#include "logicalaccess/logs.hpp"

#ifdef __unix__
#include <wintypes.h>
#endif

namespace logicalaccess
{
	#define EXCEPTION_MSG_AUTHENTICATE		"Authenticate failed." /**< \brief The authentication exception message */
	#define EXCEPTION_MSG_LOADKEY			"Load key failed." /**< \brief The load key exception message */
	#define EXCEPTION_MSG_NOKEY				"No key found." /**< \brief The no key found exception message */
	#define EXCEPTION_MSG_READ				"Read operation failed." /**< \brief The read exception message */
	#define EXCEPTION_MSG_WRITE				"Write operation failed." /**< \brief The write exception message */
	#define EXCEPTION_MSG_CHANGEKEY			"Change key failed." /**< \brief The change key exception message */


	class Chip;

	/**
	 * \brief The card services.
	 */
	typedef enum {
		CST_ACCESS_CONTROL = 0x0000, /**< The access control card service type */
		CST_NFC_TAG = 0x0001, /**< The NFC Tag card service type */
		CST_STORAGE = 0x0002 /**< The storage card service type */
	} CardServiceType;

	/**
	 * \brief The base card service class for all services.
	 */
	class LIBLOGICALACCESS_API CardService : public boost::enable_shared_from_this<CardService>
	{
		public:

			/**
			 * \brief Constructor.
			 * \param chip The associated chip.
			 */
			CardService(boost::shared_ptr<Chip> chip);

			/*
			 * \brief Get the associated chip object.
			 * \return The chip.
			 */
			boost::shared_ptr<Chip> getChip() { return d_chip; };

			/**
			 * \brief Get the card service type.
			 * \return The card service type.
			 */
			virtual CardServiceType getServiceType() const = 0;

		protected:

			/**
			 * \brief Chip object.
			 */
			boost::shared_ptr<Chip> d_chip;
	};
}

#endif