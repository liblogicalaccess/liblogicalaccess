/**
 * \file mifareplussl3commands.hpp
 * \author Xavier S. <xavier.schmerber@gmail.com>
 * \brief MifarePlus SL3 commands.
 */

#ifndef LOGICALACCESS_MIFAREPLUSSL3COMMANDS_HPP
#define LOGICALACCESS_MIFAREPLUSSL3COMMANDS_HPP

#include "mifarepluscommands.hpp"
#include "mifareplusaccessinfo.hpp"


namespace logicalaccess
{
	/**
	 * \brief The MifarePlus SL3 commands class.
	 */
	class LIBLOGICALACCESS_API MifarePlusSL3Commands : public virtual MifarePlusCommands
	{
		public:

			
			/**
			* \brief Write a sector.
			* \param sector The sector number, from 0 to 15.
			* \param start_block The forst block to read
			* \param buf Return value, will contain the data of the sector on success, null otherwise.
			* \param buflen The buffer length, wich defines the number of block to read
			* \param key The key to use to authenticate to the sector
			* \param keytype The key type
			*/
			virtual void writeSector(int sector, int start_block, const void* buf, size_t buflen, boost::shared_ptr<MifarePlusKey> key, MifarePlusKeyType keytype);
			
			/**
			 * \brief write several sectors.
			 * \param start_sector The start sector number, from 0 to stop_sector.
			 * \param stop_sector The stop sector number
			 * \param buf The data to write
			 * \param buflen The buffer length
			 * \param aiToUse The access informations for authentification on the sectors to write
			*/
			virtual void writeSectors(int start_sector, int stop_sector, const void* buf, size_t buflen, boost::shared_ptr<AccessInfo> aiToUse);

			/**
			 * \brief Read a whole sector.
			 * \param sector The sector number, from 0 to 15.
			 * \param start_block The forst block to read
			 * \param buf Return value, will contain the data of the sector on success, null otherwise.
			 * \param buflen The buffer length, wich defines the number of block to read
			 * \param key The key to use to authenticate to the sector
			 * \param keytype The key type
			 */
			virtual void readSector(int sector, int start_block, void* buf, size_t buflen, boost::shared_ptr<MifarePlusKey> key, MifarePlusKeyType keytype);
			
			/**
			 * \brief Read several sectors.
			 * \param start_sector The start sector number, from 0 to stop_sector.
			 * \param stop_sector The stop sector number
			 * \param buf Return value, the buffer filled with the data on success.
			 * \param buflen The buffer length
			 * \param aiToUse The access informations for authentification on the sectors to read
			*/
			virtual void readSectors(int start_sector, int stop_sector, void* buf, size_t buflen, boost::shared_ptr<AccessInfo> aiToUse);
			
			/**
			 * \brief Authenticate on a given sector
			 * \param location The location with pointed sector.
			 * \param AccessInfo The access infos containing keys for authentication.
			 * \param ret Return value, true on success, false otherwise.
			 */
			virtual bool authenticate(boost::shared_ptr<Location> location, boost::shared_ptr<AccessInfo> ai);

			/**
			 * \brief Get number of data blocks for a sector.
			 * \param sector The sector.
			 * \return The number of blocks.
			 */
			static unsigned char getNbBlocks(int sector);

			static unsigned short getBlockNo(int sector, int block);

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

#endif /* LOGICALACCESS_MIFAREPLUSSL3COMMANDS_HPP*/
