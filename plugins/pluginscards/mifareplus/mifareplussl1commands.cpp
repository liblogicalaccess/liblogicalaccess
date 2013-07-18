/**
 * \file mifareplussl1commands.cpp
 * \author Xavier SCHMERBER <xavier.schmerber@gmail.com>
 * \brief MifarePlus SL1 commands.
 */

#include "mifareplussl3commands.hpp"
#include "mifarepluschip.hpp"

#define PREFIX_PATTERN 0xE3
#define POLYNOM_PATTERN 0x1D
	
namespace logicalaccess
{
	bool MifarePlusSL1Commands::authenticate(boost::shared_ptr<Location> location, boost::shared_ptr<AccessInfo> ai)
	{
		EXCEPTION_ASSERT_WITH_LOG(location, std::invalid_argument, "location cannot be null.");
		EXCEPTION_ASSERT_WITH_LOG(ai, std::invalid_argument, "ai cannot be null.");

		boost::shared_ptr<MifarePlusLocation> mLocation = boost::dynamic_pointer_cast<MifarePlusLocation>(location);
		boost::shared_ptr<MifarePlusAccessInfo> mAi = boost::dynamic_pointer_cast<MifarePlusAccessInfo>(ai);

		EXCEPTION_ASSERT_WITH_LOG(mLocation, std::invalid_argument, "location must be a MifarePlusLocation.");
		EXCEPTION_ASSERT_WITH_LOG(mAi, std::invalid_argument, "ai must be a MifarePlusAccessInfo.");
		
		getChip()->getProfile()->clearKeys();

		EXCEPTION_ASSERT_WITH_LOG((!mAi->keyA->isEmpty()) || (!mAi->keyB->isEmpty()), std::invalid_argument, "ai must contain at least one key");

		MifarePlusKeyType keytype = KT_KEY_CRYPTO1_A;

		if (!mAi->keyA->isEmpty())
		{
			boost::dynamic_pointer_cast<MifarePlusSL1Profile>(getChip()->getProfile())->setKey(mLocation->sector, KT_KEY_CRYPTO1_A, mAi->keyA);
		}

		if (!mAi->keyB->isEmpty())
		{
			boost::dynamic_pointer_cast<MifarePlusSL1Profile>(getChip()->getProfile())->setKey(mLocation->sector, KT_KEY_CRYPTO1_B, mAi->keyB);
			keytype = KT_KEY_CRYPTO1_B;
		}

		if (!boost::dynamic_pointer_cast<MifarePlusSL1Profile>(getChip()->getProfile())->getKeyUsage(mLocation->sector, keytype))
		{
			throw EXCEPTION(CardException, EXCEPTION_MSG_NOKEY);
		}

		boost::shared_ptr<MifarePlusKey> key = boost::dynamic_pointer_cast<MifarePlusKey>(boost::dynamic_pointer_cast<MifarePlusSL1Profile>(getChip()->getProfile())->getKey(mLocation->sector, keytype));
		loadKey(location, key, keytype);
		getMifarePlusChip()->getMifarePlusSL1Commands()->authenticate(static_cast<unsigned char>(getSectorStartBlock(mLocation->sector)), key->getKeyStorage(), keytype);

		return true;
	}
	unsigned int MifarePlusSL1Commands::getSectorFromMAD(long aid, boost::shared_ptr<MifarePlusKey> madKeyA)
	{
		unsigned int sector = static_cast<unsigned int>(-1);
		MifarePlusAccessInfo::SectorAccessBits sab;
		
		if (!madKeyA->isEmpty())
		{
			getMifarePlusChip()->getMifarePlusSL1Profile()->setKey(0, KT_KEY_CRYPTO1_A, madKeyA);
		}

		unsigned char madbuf[32];
		memset(madbuf, 0x00, sizeof(madbuf));

		if (!readSector(0, 1, madbuf, sizeof(madbuf), sab))
		{
			THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Can't read the MAD.");
		}
		
		unsigned char madcrc = calculateMADCrc(madbuf, sizeof(madbuf));
		if (madcrc != madbuf[0])
		{
			THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Bad MAD CRC.");
		}
		
		sector = findReferencedSector(aid, madbuf, sizeof(madbuf));

		if ((sector == static_cast<unsigned int>(-1)) && (getChip()->getCardType() == "MifarePlus4K" || getChip()->getCardType() == "MifarePlus2K"))
		{
			unsigned char madbuf2[48];
			memset(madbuf2, 0x00, sizeof(madbuf2));

			if (readSector(16, 0, madbuf2, sizeof(madbuf2), sab) != sizeof(madbuf2))
			{
				THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Can't read the MAD2.");
			}

			unsigned char mad2crc = calculateMADCrc(madbuf2, sizeof(madbuf2));
			if (mad2crc != madbuf2[0])
			{
				THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Bad MAD2 CRC.");
			}

			sector = findReferencedSector(aid, madbuf2, sizeof(madbuf2));

			if (sector != static_cast<unsigned int>(-1))
			{
				sector += 16;
			}
		}

		return sector;
	}

	void MifarePlusSL1Commands::setSectorToMAD(long aid, unsigned int sector, boost::shared_ptr<MifarePlusKey> madKeyA, boost::shared_ptr<MifarePlusKey> madKeyB)
	{
		MifarePlusAccessInfo::SectorAccessBits sab;

		if (!madKeyA->isEmpty())
		{
			getMifarePlusChip()->getMifarePlusSL1Profile()->setKey(0, KT_KEY_CRYPTO1_A, madKeyA);
			getMifarePlusChip()->getMifarePlusSL1Profile()->setKey(16, KT_KEY_CRYPTO1_A, madKeyA);
		}

		if (madKeyB->isEmpty())
		{
			sab.setTransportConfiguration();
		}
		else
		{
			getMifarePlusChip()->getMifarePlusSL1Profile()->setKey(0, KT_KEY_CRYPTO1_B, madKeyB);
			getMifarePlusChip()->getMifarePlusSL1Profile()->setKey(16, KT_KEY_CRYPTO1_B, madKeyB);
			sab.setAReadBWriteConfiguration();
		}

		if (sector < 16)
		{
			if (sector == 0)
			{
				THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Can't make reference to the MAD itself.");
			}

			unsigned char madbuf[32];
			memset(madbuf, 0x00, sizeof(madbuf));

			if (readSector(0, 1, madbuf, sizeof(madbuf), sab) != sizeof(madbuf))
			{
				THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Can't read the MAD.");
			}
			
			madbuf[sector*2] = (aid & 0xff00) >> 8;
			madbuf[(sector*2) + 1] = aid & 0xff;
			if (madbuf[1] == 0x00)
			{
				madbuf[1] = static_cast<unsigned char>(0x01 << (sector-1));
			}
			madbuf[0] = calculateMADCrc(madbuf, sizeof(madbuf));

			if (!writeSector(0, 1, madbuf, sizeof(madbuf), sab))
			{
				THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Can't write the MAD.");
			}
		}
		else
		{
			if (sector == 16)
			{
				THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Can't make reference to the MAD2 itself.");
			}

			unsigned char madbuf2[48];
			memset(madbuf2, 0x00, sizeof(madbuf2));

			if (readSector(16, 0, madbuf2, sizeof(madbuf2), sab) != sizeof(madbuf2))
			{
				THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Can't read the MAD2.");
			}

			sector -= 16;
			madbuf2[sector*2] = (aid & 0xff00) >> 8;
			madbuf2[(sector*2) + 1] = aid & 0xff;
			madbuf2[0] = calculateMADCrc(madbuf2, sizeof(madbuf2));

			if (!writeSector(16,0, madbuf2, sizeof(madbuf2), sab))
			{
				THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Can't write the MAD2.");
			}
		}
	}

	unsigned char MifarePlusSL1Commands::calculateMADCrc(const unsigned char* madbuf, size_t madbuflen)
	{
		/* x^8 + x^4 + x^3 + x^2 + 1 => 0x11d */
		const unsigned char poly = 0x1d;

		unsigned char crc = 0xC7;
		for (unsigned int i = 1; i < madbuflen; ++i)
		{			
			crc ^= madbuf[i];
			for (int current_bit = 7; current_bit >= 0; current_bit--)
			{
				int bit_out = crc & 0x80;
				crc <<= 1;
				if (bit_out)
				{
					crc ^= poly;
				}
			}
		}

		return crc;
	}

	unsigned int MifarePlusSL1Commands::findReferencedSector(long aid, unsigned char* madbuf, size_t madbuflen)
	{
		unsigned int sector = static_cast<unsigned int>(-1);

		for (unsigned int i = 1; i < madbuflen && (sector == static_cast<unsigned int>(-1)); ++i)
		{
			long paid = 0;
			paid = (madbuf[i*2] << 8) | madbuf[(i*2) + 1];

			if (paid == aid)
			{
				sector = i;
			}
		}

		return sector;
	}

	boost::shared_ptr<MifarePlusChip> MifarePlusSL1Commands::getMifarePlusChip()
	{
		return boost::dynamic_pointer_cast<MifarePlusChip>(getChip());
	}

	void MifarePlusSL1Commands::changeBlock(const MifarePlusAccessInfo::SectorAccessBits& sab, int sector, int block, bool write)
	{
		MifarePlusKeyType wkt = KT_KEY_CRYPTO1_B;
		MifarePlusKeyType rkt = KT_KEY_CRYPTO1_A;

		int virtualblock = 0;
		if (sector >= 32)
		{
			if (block < 5)
			{
				virtualblock = 0;
			}
			else if(block < 10)
			{
				virtualblock = 1;
			}
			else
			{
				virtualblock = 2;
			}
		}
		else
		{
			virtualblock = block;
		}

		if (sab.d_data_blocks_access_bits[virtualblock].c1 && sab.d_data_blocks_access_bits[virtualblock].c2 && sab.d_data_blocks_access_bits[virtualblock].c3)
		{
			// Never read/write access
		}
		else if (!sab.d_data_blocks_access_bits[virtualblock].c3)
		{
			rkt = KT_KEY_CRYPTO1_A;
			if (!sab.d_data_blocks_access_bits[virtualblock].c1 && !sab.d_data_blocks_access_bits[virtualblock].c2)
			{
				wkt = KT_KEY_CRYPTO1_A;
			}
			else if (sab.d_data_blocks_access_bits[virtualblock].c1 && !sab.d_data_blocks_access_bits[virtualblock].c2)
			{
				wkt = KT_KEY_CRYPTO1_B;
			}
			else
			{
				// Never write access
			}
		}
		else
		{
			rkt = KT_KEY_CRYPTO1_B;
			if (!sab.d_data_blocks_access_bits[virtualblock].c1 && sab.d_data_blocks_access_bits[virtualblock].c2)
			{
				wkt = KT_KEY_CRYPTO1_B;
			}
			else
			{
				// Never write access
			}
		}

		MifarePlusKeyType keytype = (write) ? wkt : rkt;
		boost::shared_ptr<MifarePlusKey> key = boost::dynamic_pointer_cast<MifarePlusSL1Profile>(getChip()->getProfile())->getKey(sector, keytype);

		if (!boost::dynamic_pointer_cast<MifarePlusSL1Profile>(getChip()->getProfile())->getKeyUsage(sector, keytype))
		{
			throw EXCEPTION(CardException, EXCEPTION_MSG_NOKEY);
		}

		boost::shared_ptr<MifarePlusLocation> location(new MifarePlusLocation());
		location->sector = sector;
		location->block = block;

		loadKey(location, key, keytype);
		authenticate(static_cast<unsigned char>(getSectorStartBlock(sector)), key->getKeyStorage(), keytype);
	}

	size_t MifarePlusSL1Commands::readSector(int sector, int start_block, void* buf, size_t buflen, const MifarePlusAccessInfo::SectorAccessBits& sab)
	{
		size_t retlen = 0;

		if (buf == NULL || buflen < (getNbBlocks(sector)-static_cast<unsigned int>(start_block))*16)
		{
			THROW_EXCEPTION_WITH_LOG(std::invalid_argument, "Bad buffer parameter. The minimum buffer's length is 48 bytes.");
		}		

		for (int i = start_block;  i< getNbBlocks(sector); i++)
		{
			changeBlock(sab, sector, i, false);
			retlen += readBinary(static_cast<unsigned char>(getSectorStartBlock(sector) + i), 16, reinterpret_cast<char*>(buf) + retlen, 16);
		}

		return retlen;
	}

	size_t MifarePlusSL1Commands::writeSector(int sector, int start_block, const void* buf, size_t buflen, const MifarePlusAccessInfo::SectorAccessBits& sab, unsigned char userbyte, MifarePlusAccessInfo::SectorAccessBits* newsab)
	{
		size_t retlen = 0;

		if (buf == NULL || buflen < (getNbBlocks(sector)-static_cast<unsigned int>(start_block))*16)
		{
			THROW_EXCEPTION_WITH_LOG(std::invalid_argument, "Bad buffer parameter. The minimum buffer's length is 48 bytes.");
		}

		for (int i = start_block; i < getNbBlocks(sector); i++)
		{
			changeBlock(sab, sector, i, true);
			retlen += updateBinary(static_cast<unsigned char>(getSectorStartBlock(sector) + i), reinterpret_cast<const char*>(buf) + retlen, 16);
		}

		if (newsab != NULL)
		{
			int writeSector = getMifarePlusChip()->getNbSectors();
			changeKey(boost::dynamic_pointer_cast<MifarePlusSL1Profile>(getChip()->getProfile())->getKey(writeSector, KT_KEY_CRYPTO1_A), boost::dynamic_pointer_cast<MifarePlusSL1Profile>(getChip()->getProfile())->getKey(writeSector, KT_KEY_CRYPTO1_B), sector, sab, newsab, userbyte);
		}

		return retlen;
	}

	void MifarePlusSL1Commands::changeKey(boost::shared_ptr<MifarePlusKey> keyA, boost::shared_ptr<MifarePlusKey> keyB, unsigned int sector, const MifarePlusAccessInfo::SectorAccessBits& sab, MifarePlusAccessInfo::SectorAccessBits* newsab, unsigned char userbyte)
	{
		char trailerblock[16];
		memset(trailerblock, 0x00, sizeof(trailerblock));

		if (!keyB || keyB->isEmpty())
		{
			memcpy(&trailerblock[0], keyA->getData(), MIFARE_PLUS_CRYPTO1_KEY_SIZE);
		} else
		{
			memcpy(&trailerblock[0], keyA->getData(), MIFARE_PLUS_CRYPTO1_KEY_SIZE);
			memcpy(&trailerblock[10], keyB->getData(), MIFARE_PLUS_CRYPTO1_KEY_SIZE);
		}

		if ((*newsab).toArray(&trailerblock[MIFARE_PLUS_CRYPTO1_KEY_SIZE], 3) != 3)
		{
			throw EXCEPTION(CardException, EXCEPTION_MSG_CHANGEKEY);
		}

		trailerblock[MIFARE_PLUS_CRYPTO1_KEY_SIZE + 3] = userbyte;

		changeBlock(sab, sector, getNbBlocks(sector), true);
		if (updateBinary(static_cast<unsigned char>(getSectorStartBlock(sector) + getNbBlocks(sector)), trailerblock, 16) != 16)
		{
			throw EXCEPTION(CardException, EXCEPTION_MSG_CHANGEKEY);
		}
	}

	size_t MifarePlusSL1Commands::readSectors(int start_sector, int stop_sector, int start_block, void* buf, size_t buflen, const MifarePlusAccessInfo::SectorAccessBits& sab)
	{
		if (start_sector > stop_sector)
		{
			THROW_EXCEPTION_WITH_LOG(std::invalid_argument, "Start sector can't be greater than stop sector.");
		}

		size_t minsize = 0;
		size_t offset = 0;

		for (int i = start_sector; i <= stop_sector; ++i)
		{
			minsize += getNbBlocks(i) * 16;
			if (buflen < (minsize - (start_block * 16)))
			{
				THROW_EXCEPTION_WITH_LOG(std::invalid_argument, "The buffer is too short.");
			}
			int startBlockSector = (i == start_sector) ? start_block : 0;
			offset += readSector(i, startBlockSector, reinterpret_cast<char*>(buf) + offset, (getNbBlocks(i)-startBlockSector)*16, sab);
		}

		return minsize;
	}

	size_t MifarePlusSL1Commands::writeSectors(int start_sector, int stop_sector, int start_block, const void* buf, size_t buflen, const MifarePlusAccessInfo::SectorAccessBits& sab, unsigned char userbyte, MifarePlusAccessInfo::SectorAccessBits* newsab)
	{
		if (start_sector > stop_sector)
		{
			THROW_EXCEPTION_WITH_LOG(std::invalid_argument, "Start sector can't be greater than stop sector.");
		}

		size_t minsize = 0;
		size_t offset = 0;

		for (int i = start_sector; i <= stop_sector; ++i)
		{
			minsize += getNbBlocks(i) * 16;
			if (buflen < (minsize - (start_block * 16)))
			{
				THROW_EXCEPTION_WITH_LOG(std::invalid_argument, "The buffer is too short.");
			}
			int startBlockSector = (i == start_sector) ? start_block : 0;
			offset += writeSector(i, startBlockSector, reinterpret_cast<const char*>(buf) + offset, (getNbBlocks(i)-startBlockSector)*16, sab, userbyte, newsab);
		}

		return minsize;
	}

	unsigned char MifarePlusSL1Commands::getNbBlocks(int sector) const
	{
		return ((sector >= 32) ? 15 : 3);
	}

	unsigned char MifarePlusSL1Commands::getSectorStartBlock(int sector) const
	{
		unsigned char start_block = 0;
		for (int i = 0; i < sector; ++i)
		{
			start_block += getNbBlocks(i) + 1;
		}

		return start_block;
	}
}