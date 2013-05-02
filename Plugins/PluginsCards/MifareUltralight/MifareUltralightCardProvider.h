/**
 * \file MifareUltralightCardProvider.h
 * \author Maxime CHAMLEY <maxime.chamley@islog.eu>
 * \brief Mifare Ultralight card interface.
 */

#ifndef LOGICALACCESS_MIFAREULTRALIGHTCARDPROVIDER_H
#define LOGICALACCESS_MIFAREULTRALIGHTCARDPROVIDER_H

#include "logicalaccess/Cards/CardProvider.h"
#include "MifareUltralightCommands.h"

#include <string>
#include <vector>
#include <iostream>

using std::string;
using std::vector;
using std::ostream;

namespace LOGICALACCESS
{	
	class MifareUltralightChip;

	/**
	 * \brief The Mifare Ultralight base profile class.
	 */
	class LIBLOGICALACCESS_API MifareUltralightCardProvider : public CardProvider
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
			 * \brief Read several pages.
			 * \param start_page The start page number, from 0 to stop_page.
			 * \param stop_page The stop page number, from start_page to 15.
			 * \param buf The buffer to fill with the data.
			 * \param buflen The length of buf. Must be at least (stop_page - start_page + 1) * 4 bytes long.
			 * \return The number of bytes red, or a negative value on error.
			 */
			virtual size_t readPages(int start_page, int stop_page, void* buf, size_t buflen);

			/**
			 * \brief Write several pages.
			 * \param start_page The start page number, from 0 to stop_page.
			 * \param stop_page The stop page number, from start_page to 15.
			 * \param buf The buffer to fill with the data.
			 * \param buflen The length of buf. Must be at least (stop_page - start_page + 1) * 4 bytes long.		 
			 * \return The number of bytes red, or a negative value on error.
			 */
			virtual size_t writePages(int start_page, int stop_page, const void* buf, size_t buflen);				

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
			 * \brief Set a page as read-only.
			 * \param page The page to lock.
			 */
			virtual void lockPage(int page);

			/*
			 * \brief Get the associated Mifare Ultralight chip.
			 * \return The Mifare Ultralight chip.
			 */
			boost::shared_ptr<MifareUltralightChip> getMifareUltralightChip();

			boost::shared_ptr<MifareUltralightCommands> getMifareUltralightCommands() const { return boost::dynamic_pointer_cast<MifareUltralightCommands>(getCommands()); };

		protected:							

	};	
}

#endif /* LOGICALACCESS_MIFAREULTRALIGHTCARDPROVIDER_H */

