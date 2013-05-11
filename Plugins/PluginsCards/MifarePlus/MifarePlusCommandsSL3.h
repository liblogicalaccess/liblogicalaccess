/**
 * \file MifarePlusCommandsSL3.h
 * \author Xavier S. <xavier.schmerber@gmail.com>
 * \brief MifarePlus SL3 commands.
 */

#ifndef LOGICALACCESS_MIFAREPLUSCOMMANDSSL3_H
#define LOGICALACCESS_MIFAREPLUSCOMMANDSSL3_H

#include "MifarePlusCommands.h"
#include "MifarePlusAccessInfo.h"


namespace logicalaccess
{
	/**
	 * \brief The MifarePlus SL3 commands class.
	 */
	class LIBLOGICALACCESS_API MifarePlusCommandsSL3 : public virtual MifarePlusCommands
	{
		public:

			/**
			 * \brief Authenticate a block, given a key number.
			 * \param sector The sector to authenticate to (null if key is not a sector key)
			 * \param key The key to use for authentication.
			 * \param keytype The mifare plus key type.
			 * \return true if authenticated, false otherwise.
			 */
			virtual bool followingAuthenticate(int sector, boost::shared_ptr<MifarePlusKey> key, MifarePlusKeyType keytype) = 0;

			/**
			 * \brief Authenticate a block, given a key number.
			 * \param sector The sector to authenticate to (null if key is not a sector key)
			 * \param key The key to use for authentication.
			 * \param keytype The mifare plus key type.
			 * \return true if authenticated, false otherwise.
			 */
			virtual bool authenticate(int sector, boost::shared_ptr<MifarePlusKey> key, MifarePlusKeyType keytype) = 0;

			/**
			* \brief Reset the authentication
			*/
			virtual void resetAuthentication() = 0;

			/**
			 * \brief Read bytes from the card.
			 * \param blockno The block number.
			 * \param len The number of blocks to read. (0 <= len < 16)
			 * \param encrypted The encryption indicator
			 * \param macOnCommand The mac on command indicator
			 * \param macOnResponse The mac on response indicator
			 * \param buf The buffer in which to place the data.
			 * \param buflen The length of buffer.
			 * \return The count of bytes red.
			 */
			virtual size_t readBinary(short blockno, char len, bool encrypted, bool macOnCommand, bool macOnResponse, void* buf, size_t buflen) = 0;

			/**
			 * \brief Read bytes from the card.
			 * \param blockno The block number.
			 * \param encrypted The encryption indicator
			 * \param macOnResponse The mac on response indicator
			 * \param buf The buffer containing the data to write
			 * \param buflen The length of buffer.
			 * \return The count of bytes red.
			 */
			virtual size_t updateBinary(short blockno, bool encrypted, bool macOnResponse, const void* buf, size_t buflen) = 0;

			/**
			 * \brief Increment a block
			 * \param blockno The block number of the block to be incremented.
			 * \param macOnResponse The mac on response indicator
			 * \param value The data
			 * \return true if operation was successfull
			 */
			virtual bool increment(short blockno, bool macOnResponse, int value) = 0;
			
			/**
			 * \brief Decrement a block
			 * \param blockno The block number of the block to be decremented.
			 * \param macOnResponse The mac on response indicator
			 * \param value The data
			 * \return true if operation was successfull
			 */
			virtual bool decrement(short blockno, bool macOnResponse, int value) = 0;

			/**
			 * \brief Transfer a block
			 * \param blockno The block number of the destination block
			 * \param macOnResponse The mac on response indicator
			 * \return true if operation was successfull
			 */
			virtual bool transfer(short blockno, bool macOnResponse) = 0;

			/**
			 * \brief Increment aand transfer a block
			 * \param sourceBlockno The block number of the source block to be incremented.
			 * \param destBlockno The block number of the destination block to be incremented.
			 * \param macOnResponse The mac on response indicator
			 * \param value The data
			 * \return true if operation was successfull
			 */
			virtual bool incrementTransfer(short sourceBlockno, short destBlockno, bool macOnResponse, int value) = 0;

			/**
			 * \brief decrement aand transfer a block
			 * \param sourceBlockno The block number of the source block to be decremented.
			 * \param destBlockno The block number of the destination block to be decremented.
			 * \param macOnResponse The mac on response indicator
			 * \param value The data
			 * \return true if operation was successfull
			 */
			virtual bool decrementTransfer(short sourceBlockno, short destBlockno, bool macOnResponse, int value) = 0;

			/**
			 * \brief Restore a block
			 * \param blockno The block number of the to be restored block
			 * \param macOnResponse The mac on response indicator
			 * \return true if operation was successfull
			 */
			virtual bool restore(short blockno, bool macOnResponse) = 0;

			/**
			* \brief Check the card proximity
			* \param auth The indicator of active session (protocol differs if card is authenticated)
			*/
			//virtual bool proximityCheck(bool auth) = 0;
			//TODO : find the PPS1 and calculate the mac to use the proximity check

		protected:
	};
}

#endif /* LOGICALACCESS_MIFAREPLUSCOMMANDSSL3_H */
