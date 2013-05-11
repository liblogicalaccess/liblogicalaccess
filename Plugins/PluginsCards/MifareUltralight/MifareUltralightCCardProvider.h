/**
 * \file MifareUltralightCCardProvider.h
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Mifare Ultralight C card interface.
 */

#ifndef LOGICALACCESS_MIFAREULTRALIGHTCCARDPROVIDER_H
#define LOGICALACCESS_MIFAREULTRALIGHTCCARDPROVIDER_H

#include "MifareUltralightCardProvider.h"
#include "MifareUltralightCCommands.h"


namespace logicalaccess
{	
	class MifareUltralightCChip;

	/**
	 * \brief The Mifare Ultralight C base profile class.
	 */
	class LIBLOGICALACCESS_API MifareUltralightCCardProvider : public MifareUltralightCardProvider
	{
		public:

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
			 * \brief Authenticate to the chip.
			 * \param aiToUse The access information to use for authentication.
			 */
			void authenticate(boost::shared_ptr<AccessInfo> aiToUse);

			/**
			 * \brief Change the chip authentication key.
			 * \param key The new key.
			 */
			virtual void changeKey(boost::shared_ptr<TripleDESKey> key);

			/**
			 * \brief Set a page as read-only.
			 * \param page The page to lock.
			 */
			virtual void lockPage(int page);

			/*
			 * \brief Get the associated Mifare Ultralight C chip.
			 * \return The Mifare Ultralight C chip.
			 */
			boost::shared_ptr<MifareUltralightCChip> getMifareUltralightCChip();

			boost::shared_ptr<MifareUltralightCCommands> getMifareUltralightCCommands() const { return boost::dynamic_pointer_cast<MifareUltralightCCommands>(getMifareUltralightCommands()); };
	};	
}

#endif /* LOGICALACCESS_MIFAREULTRALIGHTCCARDPROVIDER_H */

