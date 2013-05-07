/**
 * \file MifarePlusCardProviderSL1.h
 * \author Xavier S. <xavier.schmerber@gmail.com>
 * \brief MifarePlus SL1 card interface.
 */

#ifndef LOGICALACCESS_MIFAREPLUSCARDPROVIDERSL1_H
#define LOGICALACCESS_MIFAREPLUSCARDPROVIDERSL1_H

#include "logicalaccess/Key.h"
#include "MifarePlusLocation.h"
#include "logicalaccess/Cards/CardProvider.h"
#include "MifarePlusCommandSL1.h"
#include "MifarePlusKey.h"
#include "MifarePlusAccessInfo.h"

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
	class LIBLOGICALACCESS_API MifarePlusCardProviderSL1 : public CardProvider
	{
		public:			

			/**
			 * \brief Constructor for MifarePlus.
			 */
			MifarePlusCardProviderSL1();

			/**
			 * \brief Destructor.
			 */
			virtual ~MifarePlusCardProviderSL1();

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
			 * \brief Read a whole sector.
			 * \param sector The sector number, from 0 to 15.
			 * \param start_block The first block on the sector.
			 * \param buf A buffer to fill with the data of the sector.
			 * \param buflen The length of buffer. Must be at least 48 bytes long or the call will fail.
			 * \param sab The sector access bits.
			 * \return The number of bytes red, or a negative value on error.
			 */
			virtual size_t readSector(int sector, int start_block, void* buf, size_t buflen, const MifarePlusAccessInfo::SectorAccessBits& sab);

			/**
			 * \brief Write a whole sector.
			 * \param sector The sector number, from 0 to 15.
			 * \param start_block The first block on the sector.
			 * \param buf A buffer to from which to copy the data.
			 * \param buflen The length of buffer. Must be at least 48 bytes long or the call will fail.
			 * \param sab The sector access bits.
			 * \param userbyte The user byte on trailer block.
			 * \param newsab If not NULL the key will be changed as well.
			 * \return The number of bytes written, or a negative value on error.
			 * \warning If sector is 0, the first 16 bytes will be skipped and be considered "copied" since the first block in sector 0 is read-only.
			 */
			virtual size_t writeSector(int sector, int start_block, const void* buf, size_t buflen, const MifarePlusAccessInfo::SectorAccessBits& sab, unsigned char userbyte = 0x00, MifarePlusAccessInfo::SectorAccessBits* newsab = NULL);

			/**
			 * \brief Read several sectors.
			 * \param start_sector The start sector number, from 0 to stop_sector.
			 * \param stop_sector The stop sector number, from start_sector to 15.
			 * \param start_block The first block on the sector.
			 * \param buf The buffer to fill with the data.
			 * \param buflen The length of buf. Must be at least (stop_sector - start_sector + 1) * 48 bytes long.
			 * \param sab The sector access bits.
			 * \return The number of bytes red, or a negative value on error.
			 */
			virtual size_t readSectors(int start_sector, int stop_sector, int start_block, void* buf, size_t buflen, const MifarePlusAccessInfo::SectorAccessBits& sab);

			/**
			 * \brief Write several sectors.
			 * \param start_sector The start sector number, from 0 to stop_sector.
			 * \param stop_sector The stop sector number, from start_sector to 15.
			 * \param start_block The first block on the sector.
			 * \param buf The buffer to fill with the data.
			 * \param buflen The length of buf. Must be at least (stop_sector - start_sector + 1) * 48 bytes long.
			 * \param sab The sector access bits.
			 * \param userbyte The user byte on trailer blocks.
			 * \param newsab If not NULL the keys will be changed as well.			 
			 * \return The number of bytes red, or a negative value on error.
			 */
			virtual size_t writeSectors(int start_sector, int stop_sector, int start_block, const void* buf, size_t buflen, const MifarePlusAccessInfo::SectorAccessBits& sab, unsigned char userbyte = 0x00, MifarePlusAccessInfo::SectorAccessBits* newsab = NULL);

			/**
			 * \brief Get the sector referenced by the AID from the MAD.
			 * \param aid The application ID.
			 * \param madKeyA The MAD key A for read access.
			 * \return The sector.
			 */
			unsigned int getSectorFromMAD(long aid, boost::shared_ptr<MifarePlusKey> madKeyA);

			/**
			 * \brief Set the sector referenced by the AID to the MAD.
			 * \param aid The application ID.
			 * \param sector The sector.
			 * \param madKeyA The MAD key A for read access.
			 * \param madKeyB The MAD key B for write access.
			 */
			void setSectorToMAD(long aid, unsigned int sector, boost::shared_ptr<MifarePlusKey> madKeyA, boost::shared_ptr<MifarePlusKey> madKeyB);

			/**
			 * \brief Calculate the MAD crc.
			 * \param madbuf The MAD buffer.
			 * \param madbuflen The MAD buffer length.
			 * \return The CRC.
			 */
			unsigned char calculateMADCrc(const unsigned char* madbuf, size_t madbuflen);

			/**
			 * \brief Find the referenced sector in the MAD.
			 * \param aid The application ID.
			 * \param madbuf The MAD buffer.
			 * \param madbuflen The MAD buffer length.
			 */
			unsigned int findReferencedSector(long aid, unsigned char* madbuf, size_t madbuflen);

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
			 * \brief Authenticate on a given location.
			 * \param location The location.
			 * \param ai The access infos.
			 */
			bool authenticate(boost::shared_ptr<Location> location, boost::shared_ptr<AccessInfo> ai);

			void changeKey(boost::shared_ptr<MifarePlusKey> keyA, boost::shared_ptr<MifarePlusKey> keyB, unsigned int sector, const MifarePlusAccessInfo::SectorAccessBits& sab, MifarePlusAccessInfo::SectorAccessBits* newsab, unsigned char userbyte = 0x00);

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

			/*
			 * \brief Get the associated Mifare chip.
			 * \return The Mifare chip.
			 */
			boost::shared_ptr<MifarePlusChip> getMifarePlusChip();

			boost::shared_ptr<MifarePlusCommandsSL1> getMifarePlusCommandsSL1() const { return boost::dynamic_pointer_cast<MifarePlusCommandsSL1>(getCommands()); };

			/**
			 * \brief Change authenticated block.
			 * \param sab The security access bits.
			 * \param sector The sector.
			 * \param block The block.
			 * \param write Write access.
			 */
			void changeBlock(const MifarePlusAccessInfo::SectorAccessBits& sab, int sector, int block, bool write);

		protected:							

			/**
			 * \brief Get number of data blocks for a sector.
			 * \param sector The sector.
			 * \return The number of blocks.
			 */
			unsigned char getNbBlocks(int sector) const;

			/**
			 * \brief Get the sector start block.
			 * \param sector The sector.
			 * \return The first block for this sector.
			 */
			unsigned char getSectorStartBlock(int sector) const;
	};
}

#endif /* LOGICALACCESS_MIFARECARDPROVIDERSL1_H */
