/**
 * \file MifareCommands.h
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Mifare commands.
 */

#ifndef LOGICALACCESS_MIFARECOMMANDS_H
#define LOGICALACCESS_MIFARECOMMANDS_H

#include "MifareAccessInfo.h"
#include "logicalaccess/Cards/Commands.h"
#include "logicalaccess/Cards/Location.h"


namespace logicalaccess
{
	/**
	 * \brief The Mifare commands class.
	 */
	class LIBLOGICALACCESS_API MifareCommands : public virtual Commands
	{
		public:

			/**
			 * \brief Read bytes from the card.
			 * \param blockno The block number.
			 * \param len The count of bytes to read. (0 <= len < 16)
			 * \param buf The buffer in which to place the data.
			 * \param buflen The length of buffer.
			 * \return The count of bytes red.
			 */
			virtual size_t readBinary(unsigned char blockno, size_t len, void* buf, size_t buflen) = 0;

			/**
			 * \brief Write bytes to the card.
			 * \param blockno The block number.
			 * \param buf The buffer containing the data.
			 * \param buflen The length of buffer.
			 * \return The count of bytes written.
			 */
			virtual size_t updateBinary(unsigned char blockno, const void* buf, size_t buflen) = 0;

			/**
			 * \brief Load a key to the reader.
			 * \param keyno The reader key slot number. Can be anything from 0x00 to 0x1F.
			 * \param keytype The key type.
			 * \param key The key byte array.
			 * \param keylen The key byte array length.
			 * \param vol Use volatile memory.
			 * \return true on success, false otherwise.
			 */
			virtual bool loadKey(unsigned char keyno, MifareKeyType keytype, const void* key, size_t keylen, bool vol = false) = 0;

			/**
			 * \brief Load a key on a given location.
			 * \param location The location.
			 * \param key The key.
			 * \param keytype The mifare key type.
			 */
			virtual void loadKey(boost::shared_ptr<Location> location, boost::shared_ptr<Key> key, MifareKeyType keytype) = 0;

			/**
			 * \brief Authenticate a block, given a key number.
			 * \param blockno The block number.
			 * \param keyno The key number, previously loaded with Mifare::loadKey().
			 * \param keytype The key type.
			 * \return true if authenticated, false otherwise.
			 */
			virtual bool authenticate(unsigned char blockno, unsigned char keyno, MifareKeyType keytype) = 0;
			
			/**
			 * \brief Authenticate a block, given a key number.
			 * \param blockno The block number.
			 * \param key_storage The key storage used for authentication.
			 * \param keytype The key type.
			 * \return true if authenticated, false otherwise.
			 */
			virtual void authenticate(unsigned char blockno, boost::shared_ptr<KeyStorage> key_storage, MifareKeyType keytype) = 0;


	protected:
			
	};
}

#endif
