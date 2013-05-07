/**
 * \file ISO7816CardProvider.h
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief ISO7816 card provider. See http://www.cardwerk.com/smartcards/smartcard_standard_ISO7816-4.aspx
 */

#ifndef LOGICALACCESS_ISO7816CARDPROVIDER_H
#define LOGICALACCESS_ISO7816CARDPROVIDER_H

#include "logicalaccess/Cards/CardProvider.h"
#include "ISO7816Commands.h"


namespace LOGICALACCESS
{
	class ISO7816Chip;

	/**
	 * \brief The ISO7816 card provider base class.
	 */
	class LIBLOGICALACCESS_API ISO7816CardProvider : public CardProvider
	{
		public:			

			/**
			 * \brief Constructor.
			 */
			ISO7816CardProvider();

			/**
			 * \brief Destructor.
			 */
			virtual ~ISO7816CardProvider();		

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
			 * \brief Write data on a specific Tag-It location, using given Tag-It keys.
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
			 * \brief Read data on a specific Tag-It location, using given Tag-It keys.
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
			 * \brief Get the associated ISO7816 chip.
			 * \return The ISO7816 chip.
			 */
			boost::shared_ptr<ISO7816Chip> getISO7816Chip();

			boost::shared_ptr<ISO7816Commands> getISO7816Commands() const { return boost::dynamic_pointer_cast<ISO7816Commands>(getCommands()); };

		protected:

	};
}

#endif
