/**
 * \file AccessControlCardService.h
 * \author Maxime CHAMLEY <maxime.chamley@islog.eu>
 * \brief Access Control Card service.
 */

#ifndef LOGICALACCESS_ACCESSCONTROLCARDSERVICE_H
#define LOGICALACCESS_ACCESSCONTROLCARDSERVICE_H

#include "logicalaccess/Services/CardService.h"
#include "logicalaccess/Services/AccessControl/Formats/Format.h"
#include "logicalaccess/Cards/Location.h"
#include "logicalaccess/Cards/AccessInfo.h"


namespace LOGICALACCESS
{
	/**
	 * \brief The base access control card service class for all access control services.
	 */
	class LIBLOGICALACCESS_API AccessControlCardService : public CardService
	{
		public:

			/**
			 * \brief Constructor.
			 * \param cardProvider The associated card provider.
			 */
			AccessControlCardService(boost::shared_ptr<CardProvider> cardProvider);

			/**
			 * \brief Destructor.
			 */
			~AccessControlCardService();

			/**
			 * \brief Get the card service type.
			 * \return The card service type.
			 */
			virtual CardServiceType getServiceType() const;

			/**
			 * \brief Read format from the card.
			 * \param format The format to read.
			 * \param location The format location.
			 * \param aiToUse The key's informations to use.
			 * \return The format read on success, null otherwise.
			 */
			virtual boost::shared_ptr<Format> readFormat(boost::shared_ptr<Format> format, boost::shared_ptr<Location> location, boost::shared_ptr<AccessInfo> aiToUse);

			/**
			 * \brief Write format to the card.
			 * \param format The format to write.
			 * \param location The format location.
			 * \param aiToUse The key's informations to use.
			 * \param aiToWrite The key's informations to write.
			 * \return True on success, false otherwise.
			 */
			virtual bool writeFormat(boost::shared_ptr<Format> format, boost::shared_ptr<Location> location, boost::shared_ptr<AccessInfo> aiToUse, boost::shared_ptr<AccessInfo> aiToWrite);

		protected:

			
	};
}

#endif