/**
 * \file GenericTagCardProvider.h
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Generic tag card provider.
 */

#ifndef LOGICALACCESS_GENERICTAGCARDPROVIDER_H
#define LOGICALACCESS_GENERICTAGCARDPROVIDER_H

#include "logicalaccess/Key.h"
#include "logicalaccess/Cards/CardProvider.h"

#include <string>
#include <vector>
#include <iostream>

using std::string;
using std::vector;
using std::ostream;

namespace LOGICALACCESS
{
	/**
	 * \brief The Generic Tag card provider class.
	 */
	class LIBLOGICALACCESS_API GenericTagCardProvider : public CardProvider
	{
		public:			

			/**
			 * \brief Constructor.
			 */
			GenericTagCardProvider();

			/**
			 * \brief Destructor.
			 */
			virtual ~GenericTagCardProvider();
			
			/**
			 * \brief Erase the card.
			 * \return true if the card was erased, false otherwise. If false, the card may be partially erased.
			 */
			virtual bool erase();

			/**
			 * \brief Erase a specific location on the card.
			 * \param location The data location.
			 * \param aiToUse The key's informations to use to delete.
			 * \return true if the card was erased, false otherwise. If false, the card may be partially erased.
			 */
			virtual bool erase(boost::shared_ptr<Location> location, boost::shared_ptr<AccessInfo> aiToUse);			

			/**
			 * \brief Write data on a specific Mifare location, using given Mifare keys.
			 * \param location The data location.
			 * \param aiToUse The key's informations to use for write access.
			 * \param aiToWrite The key's informations to change.
			 * \param data Data to write.
			 * \param dataLength Data's length to write.
			 * \param behaviorFlags Flags which determines the behavior.
			 * \return True on success, false otherwise.
			 */
			virtual bool writeData(boost::shared_ptr<Location> location, boost::shared_ptr<AccessInfo> aiToUse, boost::shared_ptr<AccessInfo> aiToWrite, const void* data, size_t dataLength, CardBehavior behaviorFlags);

			/**
			 * \brief Read data on a specific Mifare location, using given Mifare keys.
			 * \param location The data location.
			 * \param aiToUse The key's informations to use for write access.
			 * \param data Will contain data after reading.
			 * \param dataLength Data's length to read.
			 * \param behaviorFlags Flags which determines the behavior.
			 * \return True on success, false otherwise.
			 */
			virtual bool readData(boost::shared_ptr<Location> location, boost::shared_ptr<AccessInfo> aiToUse, void* data, size_t dataLength, CardBehavior behaviorFlags);
			
			/**
			 * \brief Read data header on a specific location, using given keys.
			 * \param location The data location.
			 * \param aiToUse The key's informations to use.
			 * \param data Will contain data after reading.
			 * \param dataLength Data's length to read.
			 * \return Data header length.
			 */
			virtual size_t readDataHeader(boost::shared_ptr<Location> location, boost::shared_ptr<AccessInfo> aiToUse, void* data, size_t dataLength);

			/**
			 * \brief Get a card service for this card provider.
			 * \param serviceType The card service type.
			 * \return The card service.
			 */
			virtual boost::shared_ptr<CardService> getService(CardServiceType serviceType);

		protected:

	};
}

#endif
