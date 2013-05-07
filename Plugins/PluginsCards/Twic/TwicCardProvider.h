/**
 * \file TwicCardProvider.h
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Twic card provider.
 */

#ifndef LOGICALACCESS_TWICCARDPROVIDER_H
#define LOGICALACCESS_TWICCARDPROVIDER_H

#include "logicalaccess/Key.h"
#include "../ISO7816/ISO7816CardProvider.h"
#include "TwicLocation.h"
#include "TwicCommands.h"

#include <string>
#include <vector>
#include <iostream>

using std::string;
using std::vector;
using std::ostream;


namespace LOGICALACCESS
{
	class TwicProfile;

	/**
	 * \brief The Twic card provider base class.
	 */
	class LIBLOGICALACCESS_API TwicCardProvider : public ISO7816CardProvider
	{
		public:			

			/**
			 * \brief Constructor.
			 */
			TwicCardProvider();

			/**
			 * \brief Destructor.
			 */
			virtual ~TwicCardProvider();

			size_t getMinimumBytesRepresentation(size_t value);

			size_t getMaximumDataObjectLength(int64_t dataObject);

			size_t getMaximumTagLength(int64_t dataObject, unsigned char tag);

			size_t getValueFromBytes(unsigned char* data, size_t datalength);

			/**
			 * \brief Get the current length for a data object.
			 * \param dataObject The data object.
			 * \param withObjectLength Add header that describe the object length to the total number, or not.
			 * \return The current data object length.
			 */
			size_t getDataObjectLength(int64_t dataObject, bool withObjectLength = false);

			bool getTagData(boost::shared_ptr<TwicLocation> location, void* data, size_t datalen, void* datatag, size_t& datataglen);			

			/**
			 * \brief Read data on a specific DESFire location, using given DESFire keys.
			 * \param location The data location.
			 * \param aiToUse The key's informations to use for write access.
			 * \param data Will contain data after reading.
			 * \param dataLength Data's length to read.
			 * \param behaviorFlags Flags which determines the behavior.
			 * \return True on success, false otherwise.
			 */
			virtual bool readData(boost::shared_ptr<Location> location, boost::shared_ptr<AccessInfo> aiToUse, void* data, size_t dataLength, CardBehavior behaviorFlags);

			/**
			 * \brief Get a card service for this card provider.
			 * \param serviceType The card service type.
			 * \return The card service.
			 */
			virtual boost::shared_ptr<CardService> getService(CardServiceType serviceType);

			boost::shared_ptr<TwicCommands> getTwicCommands() const { return boost::dynamic_pointer_cast<TwicCommands>(getCommands()); };

		protected:

			
	};
}

#endif
