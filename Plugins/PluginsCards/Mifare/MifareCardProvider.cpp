/**
 * \file MifareCardProvider.cpp
 * \author Julien KAUFFMANN <julien.kauffmann@islog.eu>, Maxime CHAMLEY <maxime.chamley@islog.eu>
 * \brief Mifare card provider.
 */

#include "MifareCardProvider.h"
#include "MifareChip.h"
#include "logicalaccess/Services/AccessControl/AccessControlCardService.h"

#define PREFIX_PATTERN 0xE3
#define POLYNOM_PATTERN 0x1D

namespace LOGICALACCESS
{	
	MifareCardProvider::MifareCardProvider()
		: CardProvider()
	{
		
	}

	MifareCardProvider::~MifareCardProvider()
	{
		
	}

	bool MifareCardProvider::authenticate(boost::shared_ptr<Location> location, boost::shared_ptr<AccessInfo> ai)
	{
		EXCEPTION_ASSERT_WITH_LOG(location, std::invalid_argument, "location cannot be null.");
		EXCEPTION_ASSERT_WITH_LOG(ai, std::invalid_argument, "ai cannot be null.");

		boost::shared_ptr<MifareLocation> mLocation = boost::dynamic_pointer_cast<MifareLocation>(location);
		boost::shared_ptr<MifareAccessInfo> mAi = boost::dynamic_pointer_cast<MifareAccessInfo>(ai);

		EXCEPTION_ASSERT_WITH_LOG(mLocation, std::invalid_argument, "location must be a MifareLocation.");
		EXCEPTION_ASSERT_WITH_LOG(mAi, std::invalid_argument, "ai must be a MifareAccessInfo.");
		
		getChip()->getProfile()->clearKeys();

		EXCEPTION_ASSERT_WITH_LOG((!mAi->keyA->isEmpty()) || (!mAi->keyB->isEmpty()), std::invalid_argument, "ai must contain at least one key");

		MifareKeyType keytype = KT_KEY_A;

		if (!mAi->keyA->isEmpty())
		{
			getMifareChip()->getMifareProfile()->setKey(mLocation->sector, KT_KEY_A, mAi->keyA);
		}

		if (!mAi->keyB->isEmpty())
		{
			getMifareChip()->getMifareProfile()->setKey(mLocation->sector, KT_KEY_B, mAi->keyB);
			keytype = KT_KEY_B;
		}

		if (!getMifareChip()->getMifareProfile()->getKeyUsage(mLocation->sector, keytype))
		{
			THROW_EXCEPTION_WITH_LOG(CardException, EXCEPTION_MSG_NOKEY);
		}

		boost::shared_ptr<MifareKey> key = getMifareChip()->getMifareProfile()->getKey(mLocation->sector, keytype);
		getMifareCommands()->loadKey(location, key, keytype);
		getMifareCommands()->authenticate(static_cast<unsigned char>(getSectorStartBlock(mLocation->sector)), key->getKeyStorage(), keytype);

		return true;
	}

	bool MifareCardProvider::erase(boost::shared_ptr<Location> location, boost::shared_ptr<AccessInfo> aiToUse)
	{
		boost::shared_ptr<MifareLocation> mLocation = boost::dynamic_pointer_cast<MifareLocation>(location);
		if (!mLocation)
		{
			return false;
		}

		unsigned int zeroblock_size = getNbBlocks(mLocation->sector) * 16;
		unsigned char *zeroblock = new unsigned char[zeroblock_size];
		memset(zeroblock, 0x00, zeroblock_size);

		boost::shared_ptr<MifareAccessInfo> _aiToWrite;
		_aiToWrite.reset(new MifareAccessInfo());
		
		if (mLocation->sector == 0)
		{
			if (mLocation->block == 0)
			{
				mLocation->block = 1;
			}
		}
		else if (mLocation->useMAD)
		{

			boost::shared_ptr<MifareLocation> madLocation(new MifareLocation());
			madLocation->sector = 0;
			madLocation->block = 1;

			boost::shared_ptr<MifareAccessInfo> madAi(new MifareAccessInfo());
			if (aiToUse)
			{
				boost::shared_ptr<MifareAccessInfo> mAiToUse = boost::dynamic_pointer_cast<MifareAccessInfo>(aiToUse);
				if (mAiToUse->useMAD)
				{
					madAi->keyA = mAiToUse->madKeyA;
					madAi->keyB = mAiToUse->madKeyB;
				}
			}

			if(madAi->keyB && !madAi->keyB->isEmpty())
			{
				madAi->sab.setAReadBWriteConfiguration();
			}

			writeData(madLocation, madAi, _aiToWrite, zeroblock, 32, CB_DEFAULT);
		}

		bool tmpuseMAD = mLocation->useMAD;
		mLocation->useMAD = false;
		bool ret = writeData(location, aiToUse, _aiToWrite, zeroblock, zeroblock_size - (mLocation->block * 16), CB_DEFAULT);
		mLocation->useMAD = tmpuseMAD;
		delete[] zeroblock;

		return ret;
	}	

	bool MifareCardProvider::writeData(boost::shared_ptr<Location> location, boost::shared_ptr<AccessInfo> aiToUse, boost::shared_ptr<AccessInfo> aiToWrite, const void* data, size_t dataLength, CardBehavior behaviorFlags)
	{
		bool ret = false;

		EXCEPTION_ASSERT_WITH_LOG(location, std::invalid_argument, "location cannot be null.");
		EXCEPTION_ASSERT_WITH_LOG(data, std::invalid_argument, "data cannot be null.");

		boost::shared_ptr<MifareLocation> mLocation = boost::dynamic_pointer_cast<MifareLocation>(location);
		boost::shared_ptr<MifareAccessInfo> mAiToUse = boost::dynamic_pointer_cast<MifareAccessInfo>(aiToUse);

		EXCEPTION_ASSERT_WITH_LOG(mLocation, std::invalid_argument, "location must be a MifareLocation.");

		if (aiToUse)
		{
			EXCEPTION_ASSERT_WITH_LOG(mAiToUse, std::invalid_argument, "aiToUse must be a MifareAccessInfo.");
		}
		else
		{
			mAiToUse = boost::dynamic_pointer_cast<MifareAccessInfo>(getChip()->getProfile()->createAccessInfo());
		}
		
		bool writeAidToMad = false;
		getChip()->getProfile()->clearKeys();

		if (mLocation->useMAD)
		{
			if (mLocation->sector == 0)
			{
				mLocation->sector = getSectorFromMAD(mLocation->aid, mAiToUse->madKeyA);
			}
			else
			{
				writeAidToMad = true;
			}
		}

		size_t totaldatalen = dataLength + (mLocation->block * 16) + mLocation->byte;
		int nbSectors = 0;
		size_t totalbuflen = 0;
		while (totalbuflen < totaldatalen)
		{
			totalbuflen += getNbBlocks(mLocation->sector + nbSectors) * 16;
			nbSectors++;
			// user data on sector trailer ?
			if (nbSectors == 1 && (totaldatalen - totalbuflen) == 1)
			{
				totalbuflen++;
			}
		}
	
		if (nbSectors >= 1)
		{
			size_t buflen = totalbuflen - (mLocation->block * 16);
			std::vector<unsigned char> dataSectors;
			dataSectors.resize(buflen, 0x00);
			memcpy(&dataSectors[0] + mLocation->byte, data, dataLength);

			if (writeAidToMad)
			{
				for (int i = mLocation->sector; i < (mLocation->sector + nbSectors); ++i)
				{
					setSectorToMAD(mLocation->aid, i, mAiToUse->madKeyA, mAiToUse->madKeyB);
				}
			}

			for (int i = 0; i < nbSectors; ++i)
			{
				if (!mAiToUse->keyA->isEmpty())
				{
					getMifareChip()->getMifareProfile()->setKey(mLocation->sector + i, KT_KEY_A, mAiToUse->keyA);
				}
				if (!mAiToUse->keyB->isEmpty())
				{
					getMifareChip()->getMifareProfile()->setKey(mLocation->sector + i, KT_KEY_B, mAiToUse->keyB);
				}
			}

			boost::shared_ptr<MifareAccessInfo> mAiToWrite;
			// Write access informations too
			if (aiToWrite)
			{
				// Sector keys will be changed after writing
				mAiToWrite = boost::dynamic_pointer_cast<MifareAccessInfo>(aiToWrite);

				EXCEPTION_ASSERT_WITH_LOG(mAiToWrite, std::invalid_argument, "mAiToWrite must be a MifareAccessInfo.");

				// MAD keys are changed now
				if (writeAidToMad && mAiToWrite->useMAD)
				{
					unsigned int madsector = (mLocation->sector <= 16) ? 0 : 16;
					MifareAccessInfo::SectorAccessBits sab;
					getMifareChip()->getMifareProfile()->setKey(madsector, KT_KEY_A, mAiToUse->madKeyA);
					if (mAiToUse->madKeyB->isEmpty())
					{
						sab.setTransportConfiguration();
					}
					else
					{
						sab.setAReadBWriteConfiguration();
						getMifareChip()->getMifareProfile()->setKey(madsector, KT_KEY_B, mAiToUse->madKeyB);
					}

					MifareAccessInfo::SectorAccessBits newsab;
					if (mAiToWrite->madKeyB->isEmpty())
					{
						newsab.setTransportConfiguration();
					}
					else
					{
						newsab.setAReadBWriteConfiguration();
					}

					if (!mAiToWrite->madKeyA->isEmpty())
					{
						getMifareChip()->getMifareProfile()->setKey(getMifareChip()->getNbSectors(), KT_KEY_A, mAiToWrite->madKeyA);
					}
				
					if (!mAiToWrite->madKeyB->isEmpty())
					{
						getMifareChip()->getMifareProfile()->setKey(getMifareChip()->getNbSectors(), KT_KEY_B, mAiToWrite->madKeyB);
					}

					changeKey(mAiToWrite->madKeyA, mAiToWrite->madKeyB, madsector, sab, &newsab, mAiToWrite->madGPB);
				}

				if (!mAiToWrite->keyA->isEmpty())
				{
					getMifareChip()->getMifareProfile()->setKey(getMifareChip()->getNbSectors(), KT_KEY_A, mAiToWrite->keyA);
				}
				
				if (!mAiToWrite->keyB->isEmpty())
				{
					getMifareChip()->getMifareProfile()->setKey(getMifareChip()->getNbSectors(), KT_KEY_B, mAiToWrite->keyB);
				}

				// No key change, set to null (to not change sab, ...)
				if (mAiToWrite->keyA->isEmpty() && mAiToWrite->keyB->isEmpty())
				{
					mAiToWrite.reset();
				}
			}

			size_t reallen;			

			if (behaviorFlags & CB_AUTOSWITCHAREA)
			{
				reallen = writeSectors(mLocation->sector,
					mLocation->sector + nbSectors - 1,
					mLocation->block,
					&dataSectors[0],
					dataSectors.size(),
					mAiToUse->sab,
					mAiToUse->gpb,
					(mAiToWrite) ? &(mAiToWrite->sab) : NULL
				);
			}
			else
			{
				reallen = writeSector(mLocation->sector,
					mLocation->block,
					&dataSectors[0],
					dataSectors.size(),
					mAiToUse->sab,
					mAiToUse->gpb,
					(mAiToWrite) ? &(mAiToWrite->sab) : NULL
				);
			}

			getMifareChip()->getMifareProfile()->setKeyUsage(getMifareChip()->getNbSectors(), KT_KEY_A, false);
			getMifareChip()->getMifareProfile()->setKeyUsage(getMifareChip()->getNbSectors(), KT_KEY_B, false);

			ret = (reallen >= buflen);
		}

		return ret;
	}

	unsigned int MifareCardProvider::getSectorFromMAD(long aid, boost::shared_ptr<MifareKey> madKeyA)
	{
		unsigned int sector = static_cast<unsigned int>(-1);

		boost::shared_ptr<MifareLocation> loc;
		boost::shared_ptr<MifareAccessInfo> ai;

		loc.reset(new MifareLocation());
		loc->sector = 0;
		loc->block = 1;
		ai.reset(new MifareAccessInfo());
		ai->keyA = madKeyA;

		unsigned char madbuf[32];
		memset(madbuf, 0x00, sizeof(madbuf));

		if (!readData(loc, ai, madbuf, sizeof(madbuf), CB_DEFAULT))
		{
			THROW_EXCEPTION_WITH_LOG(LibLOGICALACCESSException, "Can't read the MAD.");
		}
		
		unsigned char madcrc = calculateMADCrc(madbuf, sizeof(madbuf));
		if (madcrc != madbuf[0])
		{
			THROW_EXCEPTION_WITH_LOG(LibLOGICALACCESSException, "Bad MAD CRC.");
		}
		
		sector = findReferencedSector(aid, madbuf, sizeof(madbuf));

		if ((sector == static_cast<unsigned int>(-1)) && getChip()->getCardType() == "Mifare4K")
		{			
			loc->sector = 16;
			loc->block = 0;
			unsigned char madbuf2[48];
			memset(madbuf2, 0x00, sizeof(madbuf2));

			if (!readData(loc, ai, madbuf2, sizeof(madbuf2), CB_DEFAULT))
			{
				THROW_EXCEPTION_WITH_LOG(LibLOGICALACCESSException, "Can't read the MAD2.");
			}

			unsigned char mad2crc = calculateMADCrc(madbuf2, sizeof(madbuf2));
			if (mad2crc != madbuf2[0])
			{
				THROW_EXCEPTION_WITH_LOG(LibLOGICALACCESSException, "Bad MAD2 CRC.");
			}

			sector = findReferencedSector(aid, madbuf2, sizeof(madbuf2));

			if (sector != static_cast<unsigned int>(-1))
			{
				sector += 16;
			}
		}

		return sector;
	}

	void MifareCardProvider::setSectorToMAD(long aid, unsigned int sector, boost::shared_ptr<MifareKey> madKeyA, boost::shared_ptr<MifareKey> madKeyB)
	{
		boost::shared_ptr<MifareLocation> loc;
		boost::shared_ptr<MifareAccessInfo> ai;

		loc.reset(new MifareLocation());
		ai.reset(new MifareAccessInfo());
		ai->keyA = madKeyA;
		ai->keyB = madKeyB;
		if (ai->keyB->isEmpty())
		{
			ai->sab.setTransportConfiguration();
		}
		else
		{
			ai->sab.setAReadBWriteConfiguration();
		}

		if (sector < 16)
		{
			if (sector == 0)
			{
				THROW_EXCEPTION_WITH_LOG(LibLOGICALACCESSException, "Can't make reference to the MAD itself.");
			}

			loc->sector = 0;
			loc->block = 1;

			unsigned char madbuf[32];
			memset(madbuf, 0x00, sizeof(madbuf));

			if (!readData(loc, ai, madbuf, sizeof(madbuf), CB_DEFAULT))
			{
				THROW_EXCEPTION_WITH_LOG(LibLOGICALACCESSException, "Can't read the MAD.");
			}
			
			madbuf[(sector*2)] = aid & 0xff;
			madbuf[sector*2 + 1] = (aid & 0xff00) >> 8;
			if (madbuf[1] == 0x00)
			{
				madbuf[1] = static_cast<unsigned char>(sector);
			}
			madbuf[0] = calculateMADCrc(madbuf, sizeof(madbuf));

			if (!writeData(loc, ai, boost::shared_ptr<AccessInfo>(), madbuf, sizeof(madbuf), CB_DEFAULT))
			{
				THROW_EXCEPTION_WITH_LOG(LibLOGICALACCESSException, "Can't write the MAD.");
			}
		}
		else
		{
			if (sector == 16)
			{
				THROW_EXCEPTION_WITH_LOG(LibLOGICALACCESSException, "Can't make reference to the MAD2 itself.");
			}

			loc->sector = 16;
			loc->block = 0;

			unsigned char madbuf2[48];
			memset(madbuf2, 0x00, sizeof(madbuf2));

			if (!readData(loc, ai, madbuf2, sizeof(madbuf2), CB_DEFAULT))
			{
				THROW_EXCEPTION_WITH_LOG(LibLOGICALACCESSException, "Can't read the MAD2.");
			}

			sector -= 16;
			madbuf2[sector*2] = (aid & 0xff00) >> 8;
			madbuf2[(sector*2) + 1] = aid & 0xff;
			madbuf2[0] = calculateMADCrc(madbuf2, sizeof(madbuf2));

			if (!writeData(loc, ai, boost::shared_ptr<AccessInfo>(), madbuf2, sizeof(madbuf2), CB_DEFAULT))
			{
				THROW_EXCEPTION_WITH_LOG(LibLOGICALACCESSException, "Can't write the MAD2.");
			}
		}
	}

	unsigned char MifareCardProvider::calculateMADCrc(const unsigned char* madbuf, size_t madbuflen)
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

	unsigned int MifareCardProvider::findReferencedSector(long aid, unsigned char* madbuf, size_t madbuflen)
	{
		unsigned int sector = static_cast<unsigned int>(-1);

		for (unsigned int i = 1; i < madbuflen && (sector == static_cast<unsigned int>(-1)); ++i)
		{
			long paid = 0;
			paid = madbuf[(i*2)] | (madbuf[i*2 + 1] << 8);

			if (paid == aid)
			{
				sector = i;
			}
		}

		return sector;
	}

	bool MifareCardProvider::readData(boost::shared_ptr<Location> location, boost::shared_ptr<AccessInfo> aiToUse, void* data, size_t dataLength, CardBehavior behaviorFlags)
	{
		bool ret = false;

		EXCEPTION_ASSERT_WITH_LOG(location, std::invalid_argument, "location cannot be null.");
		EXCEPTION_ASSERT_WITH_LOG(data, std::invalid_argument, "data cannot be null.");

		boost::shared_ptr<MifareLocation> mLocation = boost::dynamic_pointer_cast<MifareLocation>(location);
		boost::shared_ptr<MifareAccessInfo> mAiToUse = boost::dynamic_pointer_cast<MifareAccessInfo>(aiToUse);

		EXCEPTION_ASSERT_WITH_LOG(mLocation, std::invalid_argument, "location must be a MifareLocation.");
		if (aiToUse)
		{
			EXCEPTION_ASSERT_WITH_LOG(mAiToUse, std::invalid_argument, "aiToUse must be a MifareAccessInfo.");
		}
		else
		{
			mAiToUse = boost::dynamic_pointer_cast<MifareAccessInfo>(getChip()->getProfile()->createAccessInfo());
		}
		
		getChip()->getProfile()->clearKeys();

		if (mLocation->useMAD)
		{
			mLocation->sector = getSectorFromMAD(mLocation->aid, mAiToUse->madKeyA);
		}

		size_t totaldatalen = dataLength + (mLocation->block * 16) + mLocation->byte;
		int nbSectors = 0;
		size_t totalbuflen = 0;
		while (totalbuflen < totaldatalen)
		{
			totalbuflen += getNbBlocks(mLocation->sector + nbSectors) * 16;
			nbSectors++;
		}

		for (int i = 0; i < nbSectors; ++i)
		{
			if (!mAiToUse->keyA->isEmpty())
			{
				getMifareChip()->getMifareProfile()->setKey(mLocation->sector + i, KT_KEY_A, mAiToUse->keyA);
			}
			if (!mAiToUse->keyB->isEmpty())
			{
				getMifareChip()->getMifareProfile()->setKey(mLocation->sector + i, KT_KEY_B, mAiToUse->keyB);
			}
		}

		if (nbSectors >= 1)
		{
			size_t buflen = totalbuflen - (mLocation->block * 16);
			std::vector<unsigned char> dataSectors;
			dataSectors.resize(buflen, 0x00);
			size_t reallen;

			if (behaviorFlags & CB_AUTOSWITCHAREA)
			{
				reallen = readSectors(mLocation->sector, mLocation->sector + nbSectors - 1, mLocation->block, dataSectors.data(), dataSectors.size(), mAiToUse->sab); 
			}
			else
			{
				reallen = readSector(mLocation->sector, mLocation->block, dataSectors.data(), dataSectors.size(), mAiToUse->sab); 
			}

			if (dataLength <= (reallen - mLocation->byte))
			{
				memcpy(static_cast<char*>(data), &dataSectors[0] + mLocation->byte, dataLength);

				ret = true;
			}
		}

		return ret;
	}

	size_t MifareCardProvider::readDataHeader(boost::shared_ptr<Location> location, boost::shared_ptr<AccessInfo> aiToUse, void* data, size_t dataLength)
	{
		if (data == NULL || dataLength == 0)
		{
			return 16;
		}

		EXCEPTION_ASSERT_WITH_LOG(location, std::invalid_argument, "location cannot be null.");

		boost::shared_ptr<MifareLocation> mLocation = boost::dynamic_pointer_cast<MifareLocation>(location);
		EXCEPTION_ASSERT_WITH_LOG(mLocation, std::invalid_argument, "location must be a MifareLocation.");

		MifareAccessInfo::SectorAccessBits sab;		

		if (aiToUse)
		{
			boost::shared_ptr<MifareAccessInfo> mAiToUse = boost::dynamic_pointer_cast<MifareAccessInfo>(aiToUse);

			EXCEPTION_ASSERT_WITH_LOG(mAiToUse, std::invalid_argument, "aiToUse must be a MifareAccessInfo");

			if (!mAiToUse->keyA->isEmpty())
			{
				getMifareChip()->getMifareProfile()->setKey(mLocation->sector, KT_KEY_A, mAiToUse->keyA);
			}

			if (!mAiToUse->keyB->isEmpty())
			{
				getMifareChip()->getMifareProfile()->setKey(mLocation->sector, KT_KEY_B, mAiToUse->keyB);
			}

			sab = mAiToUse->sab;
		}
		else
		{
			getMifareChip()->getMifareProfile()->setDefaultKeysAt(mLocation->sector);
			sab.setTransportConfiguration();
		}

		if (dataLength >= 16)
		{
			changeBlock(sab, mLocation->sector, getNbBlocks(mLocation->sector), false);
			return getMifareCommands()->readBinary(static_cast<unsigned char>(getSectorStartBlock(mLocation->sector) + getNbBlocks(mLocation->sector)), 16, data, 16);
		}
		return 0;
	}	

	boost::shared_ptr<MifareChip> MifareCardProvider::getMifareChip()
	{
		return boost::dynamic_pointer_cast<MifareChip>(getChip());
	}

	bool MifareCardProvider::erase()
	{
		unsigned char zeroblock[16];
		unsigned char trailerblock[16];

#ifdef _LICENSE_SYSTEM
		if (!d_license.hasWriteDataAccess())
		{
			THROW_EXCEPTION_WITH_LOG(LibLOGICALACCESSException, EXCEPTION_MSG_LICENSENOACCESS);
		}
#endif

		memset(zeroblock, 0x00, 16);
		memset(trailerblock, 0xFF, 16);

		MifareAccessInfo::SectorAccessBits sab;
		sab.setTransportConfiguration();
		trailerblock[9] = 0x00;

		if (!sab.toArray(&trailerblock[6], 3))
		{
			THROW_EXCEPTION_WITH_LOG(std::invalid_argument, "Bad sector access bits configuration.");
		}
		
		bool result = false;

		for (unsigned int i = 0; i < getMifareChip()->getNbSectors(); ++i)
		{
			bool erased = true;
			bool used = false;
			unsigned int firstBlock = (i == 0) ? 1 : 0; // Don't write the first block in sector 0

			boost::shared_ptr<MifareLocation> location(new MifareLocation());
			location->sector = i;
			if (getMifareChip()->getMifareProfile()->getKeyUsage(i, KT_KEY_B))
			{
				used = true;

				boost::shared_ptr<MifareKey> key = getMifareChip()->getMifareProfile()->getKey(i, KT_KEY_B);
				
				getMifareCommands()->loadKey(location, key, KT_KEY_B);
				getMifareCommands()->authenticate(static_cast<unsigned char>(getSectorStartBlock(i)), key->getKeyStorage(), KT_KEY_B);
				
				for (unsigned int j = firstBlock; j < getNbBlocks(i); ++j)
				{
					if ((i == 0) && (j == 0))
					{
						continue;
					}

					if (!getMifareCommands()->updateBinary(static_cast<unsigned char>(getSectorStartBlock(i) + j), zeroblock, 16))
					{
						erased = false;
					}
				}

				if (!getMifareCommands()->updateBinary(static_cast<unsigned char>(getSectorStartBlock(i) + getNbBlocks(i)), trailerblock, 16))
				{
					erased = false;
				}
			}

			if ((!erased || !used) && getMifareChip()->getMifareProfile()->getKeyUsage(i, KT_KEY_A))
			{
				used = true;

				boost::shared_ptr<MifareKey> key = getMifareChip()->getMifareProfile()->getKey(i, KT_KEY_A);

				getMifareCommands()->loadKey(location, key, KT_KEY_A);
				getMifareCommands()->authenticate(static_cast<unsigned char>(getSectorStartBlock(i)), key->getKeyStorage(), KT_KEY_A);				
			
				for (unsigned int j = firstBlock; j < getNbBlocks(i); ++j)
				{
					if ((i == 0) && (j == 0))
					{
						continue;
					}

					if (!getMifareCommands()->updateBinary(static_cast<unsigned char>(getSectorStartBlock(i) + j), zeroblock, 16))
					{
						erased = false;
					}
				}

				if (!getMifareCommands()->updateBinary(static_cast<unsigned char>(getSectorStartBlock(i) + getNbBlocks(i)), trailerblock, 16))
				{
					erased = false;
				}
			}
			
			if (!erased && used)
			{
				return false;
			} else
			{
				result = true;
			}
		}

		return result;
	}

	void MifareCardProvider::changeBlock(const MifareAccessInfo::SectorAccessBits& sab, int sector, int block, bool write)
	{
		MifareKeyType wkt = KT_KEY_A;
		MifareKeyType rkt = KT_KEY_A;

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
			rkt = KT_KEY_A;
			if (!sab.d_data_blocks_access_bits[virtualblock].c1 && !sab.d_data_blocks_access_bits[virtualblock].c2)
			{
				wkt = KT_KEY_A;
			}
			else if (sab.d_data_blocks_access_bits[virtualblock].c1 && !sab.d_data_blocks_access_bits[virtualblock].c2)
			{
				wkt = KT_KEY_B;
			}
			else
			{
				// Never write access
			}
		}
		else
		{
			rkt = KT_KEY_B;
			if (!sab.d_data_blocks_access_bits[virtualblock].c1 && sab.d_data_blocks_access_bits[virtualblock].c2)
			{
				wkt = KT_KEY_B;
			}
			else
			{
				// Never write access
			}
		}

		MifareKeyType keytype = (write) ? wkt : rkt;
		boost::shared_ptr<MifareKey> key = getMifareChip()->getMifareProfile()->getKey(sector, keytype);

		if (!getMifareChip()->getMifareProfile()->getKeyUsage(sector, keytype))
		{
			THROW_EXCEPTION_WITH_LOG(CardException, EXCEPTION_MSG_NOKEY);
		}

		boost::shared_ptr<MifareLocation> location(new MifareLocation());
		location->sector = sector;
		location->block = block;

		getMifareCommands()->loadKey(location, key, keytype);
		getMifareCommands()->authenticate(static_cast<unsigned char>(getSectorStartBlock(sector)), key->getKeyStorage(), keytype);
	}

	
	size_t MifareCardProvider::readSector(int sector, int start_block, void* buf, size_t buflen, const MifareAccessInfo::SectorAccessBits& sab)
	{
#ifdef _LICENSE_SYSTEM
		if (!d_license.hasReadDataAccess())
		{
			THROW_EXCEPTION_WITH_LOG(LibLOGICALACCESSException, EXCEPTION_MSG_LICENSENOACCESS);
		}
#endif

		size_t retlen = 0;

		if (buf == NULL || buflen < (getNbBlocks(sector)-static_cast<unsigned int>(start_block))*16)
		{
			THROW_EXCEPTION_WITH_LOG(std::invalid_argument, "Bad buffer parameter. The minimum buffer's length is 48 bytes.");
		}		

		for (int i = start_block; i < getNbBlocks(sector); i++)
		{
			changeBlock(sab, sector, i, false);
			retlen += getMifareCommands()->readBinary(static_cast<unsigned char>(getSectorStartBlock(sector) + i), 16, reinterpret_cast<char*>(buf) + retlen, 16);
		}

		return retlen;
	}

	size_t MifareCardProvider::writeSector(int sector, int start_block, const void* buf, size_t buflen, const MifareAccessInfo::SectorAccessBits& sab, unsigned char userbyte, MifareAccessInfo::SectorAccessBits* newsab)
	{
#ifdef _LICENSE_SYSTEM
		if (!d_license.hasWriteDataAccess())
		{
			THROW_EXCEPTION_WITH_LOG(LibLOGICALACCESSException, EXCEPTION_MSG_LICENSENOACCESS);
		}
#endif

		size_t retlen = 0;

		if (buf == NULL || buflen < (getNbBlocks(sector)-static_cast<unsigned int>(start_block))*16)
		{
			THROW_EXCEPTION_WITH_LOG(std::invalid_argument, "Bad buffer parameter. The minimum buffer's length is 48 bytes.");
		}

		for (int i = start_block; i < getNbBlocks(sector); i++)
		{
			changeBlock(sab, sector, i, true);
			retlen += getMifareCommands()->updateBinary(static_cast<unsigned char>(getSectorStartBlock(sector) + i), reinterpret_cast<const char*>(buf) + retlen, 16);
		}

		if (newsab != NULL)
		{
			int writeSector = getMifareChip()->getNbSectors();
			changeKey(getMifareChip()-> getMifareProfile()->getKey(writeSector, KT_KEY_A), getMifareChip()-> getMifareProfile()->getKey(writeSector, KT_KEY_B), sector, sab, newsab, userbyte);
		}

		return retlen;
	}

	void MifareCardProvider::changeKey(boost::shared_ptr<MifareKey> keyA, boost::shared_ptr<MifareKey> keyB, unsigned int sector, const MifareAccessInfo::SectorAccessBits& sab, MifareAccessInfo::SectorAccessBits* newsab, unsigned char userbyte)
	{
		char trailerblock[16];
		memset(trailerblock, 0x00, sizeof(trailerblock));

		if (!keyB || keyB->isEmpty())
		{
			memcpy(&trailerblock[0], keyA->getData(), MIFARE_KEY_SIZE);
		} else
		{
			memcpy(&trailerblock[0], keyA->getData(), MIFARE_KEY_SIZE);
			memcpy(&trailerblock[10], keyB->getData(), MIFARE_KEY_SIZE);
		}

		if ((*newsab).toArray(&trailerblock[MIFARE_KEY_SIZE], 3) != 3)
		{
			THROW_EXCEPTION_WITH_LOG(CardException, EXCEPTION_MSG_CHANGEKEY);
		}

		trailerblock[MIFARE_KEY_SIZE + 3] = userbyte;

		changeBlock(sab, sector, getNbBlocks(sector), true);
		if (getMifareCommands()->updateBinary(static_cast<unsigned char>(getSectorStartBlock(sector) + getNbBlocks(sector)), trailerblock, 16) != 16)
		{
			THROW_EXCEPTION_WITH_LOG(CardException, EXCEPTION_MSG_CHANGEKEY);
		}
	}


	size_t MifareCardProvider::readSectors(int start_sector, int stop_sector, int start_block, void* buf, size_t buflen, const MifareAccessInfo::SectorAccessBits& sab)
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

	size_t MifareCardProvider::writeSectors(int start_sector, int stop_sector, int start_block, const void* buf, size_t buflen, const MifareAccessInfo::SectorAccessBits& sab, unsigned char userbyte, MifareAccessInfo::SectorAccessBits* newsab)
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

	unsigned char MifareCardProvider::getNbBlocks(int sector) const
	{
		return ((sector >= 32) ? 15 : 3);
	}

	unsigned char MifareCardProvider::getSectorStartBlock(int sector) const
	{
		unsigned char start_block = 0;
		for (int i = 0; i < sector; ++i)
		{
			start_block += getNbBlocks(i) + 1;
		}

		return start_block;
	}

	boost::shared_ptr<CardService> MifareCardProvider::getService(CardServiceType serviceType)
	{
		boost::shared_ptr<CardService> service;

		switch (serviceType)
		{
		case CST_ACCESS_CONTROL:
			{
				service.reset(new AccessControlCardService(shared_from_this()));
			}
			break;
		case CST_NFC_TAG:
		  break;
		}

		if (!service)
		{
			service = CardProvider::getService(serviceType);
		}

		return service;
	}
}

