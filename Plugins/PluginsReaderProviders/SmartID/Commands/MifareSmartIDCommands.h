/**
 * \file MifareSmartIDCommands.h
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Mifare SmartID card.
 */

#ifndef LOGICALACCESS_MIFARESMARTIDCOMMANDS_H
#define LOGICALACCESS_MIFARESMARTIDCOMMANDS_H

#include "MifareCommands.h"
#include "../ReaderCardAdapters/MifareSmartIDReaderCardAdapter.h"

#include <string>
#include <vector>
#include <iostream>

using std::string;
using std::vector;
using std::ostream;

namespace logicalaccess
{
	/**
	 * \brief The Mifare commands class for SmartID reader.
	 */
	class LIBLOGICALACCESS_API MifareSmartIDCommands : public MifareCommands
	{
		public:			

			/**
			 * \brief Constructor.
			 */
			MifareSmartIDCommands();

			/**
			 * \brief Destructor.
			 */
			virtual ~MifareSmartIDCommands();			

			/**
			 * \brief Get the SmartID reader/card adapter.
			 * \return The SmartID reader/card adapter.
			 */
			boost::shared_ptr<MifareSmartIDReaderCardAdapter> getMifareSmartIDReaderCardAdapter() const { return boost::dynamic_pointer_cast<MifareSmartIDReaderCardAdapter>(getReaderCardAdapter()); };
		
			/**
			 * \brief Load a key on a given location.
			 * \param location The location.
			 * \param key The key.
			 * \param keytype The mifare key type.
			 */
			virtual void loadKey(boost::shared_ptr<Location> location, boost::shared_ptr<Key> key, MifareKeyType keytype);

			/**
			 * \brief Authenticate a block, given a key number.
			 * \param blockno The block number.
			 * \param key_storage The key storage used for authentication.
			 * \param keytype The key type.
			 */
			virtual void authenticate(unsigned char blockno, boost::shared_ptr<KeyStorage> key_storage, MifareKeyType keytype);
			
			/**
			 * \brief Load a key to the reader.
			 * \param keyno The key number.
			 * \param keytype The key type.
			 * \param key The key byte array.
			 * \param keylen The key byte array length.
			 * \param vol Use volatile memory.
			 * \return true on success, false otherwise.
			 */
			virtual bool loadKey(unsigned char keyno, MifareKeyType keytype, const void* key, size_t keylen, bool vol = false);
			/**
			 * \brief Authenticate a block, given a key number.
			 * \param blockno The block number.
			 * \param keyno The key number, previously loaded with Mifare::loadKey().
			 * \param keytype The key type.
			 * \return true if authenticated, false otherwise.
			 */
			virtual bool authenticate(unsigned char blockno, unsigned char keyno, MifareKeyType keytype);

			/**
			 * \brief Read bytes from the card.
			 * \param blockno The block number.
			 * \param len The count of bytes to read. (0 <= len < 16)
			 * \param buf The buffer in which to place the data.
			 * \param buflen The length of buffer.
			 * \return The count of bytes red.
			 */
			virtual size_t readBinary(unsigned char blockno, size_t len, void* buf, size_t buflen);

			/**
			 * \brief Write bytes to the card.
			 * \param blockno The block number.
			 * \param buf The buffer containing the data.
			 * \param buflen The length of buffer.
			 * \return The count of bytes written.
			 */
			virtual size_t updateBinary(unsigned char blockno, const void* buf, size_t buflen);
	};	
}

#endif /* LOGICALACCESS_MIFARESMARTIDCARDPROVIDER_H */

