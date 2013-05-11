/**
 * \file ProxAccessControlCardService.h
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Prox Access Control Card service.
 */

#ifndef LOGICALACCESS_PROXACCESSCONTROLCARDSERVICE_H
#define LOGICALACCESS_PROXACCESSCONTROLCARDSERVICE_H

#include "logicalaccess/Services/AccessControl/AccessControlCardService.h"


namespace logicalaccess
{
	class ProxCardProvider;

	/**
	 * \brief The Prox access control card service class.
	 */
	class LIBLOGICALACCESS_API ProxAccessControlCardService : public AccessControlCardService
	{
		public:

			/**
			 * \brief Constructor.
			 * \param cardProvider The associated card provider.
			 */
			ProxAccessControlCardService(boost::shared_ptr<CardProvider> cardProvider);

			/**
			 * \brief Destructor.
			 */
			~ProxAccessControlCardService();

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

			/**
			 * \brief Get the PACS bits of the card.
			 * \return The PACS bits.
			 */
			std::vector<unsigned char> getPACSBits();

			boost::shared_ptr<ProxCardProvider> getProxCardProvider();

		protected:

	};
}

#endif