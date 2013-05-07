/**
 * \file MifarePlusCardProviderSL3.h
 * \author Xavier S. <xavier.schmerber@gmail.com>
 * \brief MifarePlus SL3 card interface.
 */

#ifndef LOGICALACCESS_MIFAREPLUSCARDPROVIDERSL3_H
#define LOGICALACCESS_MIFAREPLUSCARDPROVIDERSL3_H

#include "logicalaccess/Key.h"
#include "MifarePlusLocation.h"
#include "logicalaccess/Cards/CardProvider.h"
#include "MifarePlusCommandsSL3.h"
#include "MifarePlusKey.h"
#include "MifarePlusAccessInfo.h"
#include "MifarePlusProfileSL3.h"

#include <string>
#include <vector>
#include <iostream>

using std::string;
using std::vector;
using std::ostream;

namespace LOGICALACCESS
{	
	class MifarePlusChip;

	/**
	 * \brief The MifarePlus base profile class.
	 */
	class LIBLOGICALACCESS_API MifarePlusCardProviderSL3 : public CardProvider
	{
		public:
			
			/**
			 * \brief Constructor for MifarePlus.
			 */
			MifarePlusCardProviderSL3();

			/**
			 * \brief Destructor.
			 */
			virtual ~MifarePlusCardProviderSL3();

			/**
			 * \brief Erase the card.
			 * \return true if the card was erased, false otherwise.
			 */
			virtual bool erase();			

			/**
			 * \brief Erase a specific location on the card.
			 * \param location The data location.
			 * \param aiToUse The key's informations to use to delete.
			 * \return true if the card was erased, false otherwise.
			 */
			virtual bool erase(boost::shared_ptr<Location> location, boost::shared_ptr<AccessInfo> aiToUse);			

			/**
			 * \brief Write data on a specific location, using given keys.
			 * \param location The data location.
			 * \param aiToUse The key's informations to use for write access.
			 * \param aiToWrite The key's informations to change.
			 * \param data Data to write.
			 * \param dataLength Data's length to write.
			 * \param behaviorFlags Flags which determines the behavior.
			 * \return True on success, false otherwise.
			 */
			virtual	bool writeData(boost::shared_ptr<Location> location, boost::shared_ptr<AccessInfo> aiToUse, boost::shared_ptr<AccessInfo> aiToWrite, const void* data, size_t dataLength, CardBehavior behaviorFlags);

			/**
			 * \brief Read data on a specific location, using given keys.
			 * \param location The data location.
			 * \param aiToUse The key's informations to use for read access.
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

		protected:

			/**
			 * \brief Get number of data blocks for a sector.
			 * \param sector The sector.
			 * \return The number of blocks.
			 */
			unsigned char getNbBlocks(int sector) const;

			unsigned short getBlockNo(int sector, int block);
	};
}

#endif /* LOGICALACCESS_MIFARECARDPROVIDERSL3_H */
