/**
 * \file MifarePlusCardProviderSL3.cpp
 * \author Xavier SCHMERBER <xavier.schmerber@gmail.com>
 * \brief MifarePlus SL3 card provider.
 */

#include "MifarePlusCardProviderSL3.h"
#include "MifarePlusChip.h"
#include "logicalaccess/Services/AccessControl/AccessControlCardService.h"
#include "MifarePlusKey.h"


#include <iostream>
#include <iomanip>
#include <sstream>
#include <cstring>

using std::endl;
using std::dec;
using std::hex;
using std::setfill;
using std::setw;
using std::ostringstream;
using std::istringstream;

namespace logicalaccess
{	
	MifarePlusCardProviderSL3::MifarePlusCardProviderSL3()
		: CardProvider()
	{
	}

	MifarePlusCardProviderSL3::~MifarePlusCardProviderSL3()
	{
	}

	bool MifarePlusCardProviderSL3::erase()
	{
		boost::shared_ptr<MifarePlusProfileSL3> profile = boost::dynamic_pointer_cast<MifarePlusProfileSL3>(getChip()->getProfile());
		unsigned int i;
		unsigned int j;
		unsigned char zeroblock[16];
		bool erased;
		bool success = true;

		memset(zeroblock, 0x00, 16);

		for (i = 0; i < boost::dynamic_pointer_cast<MifarePlusChip>(getChip())->getNbSectors(); ++i)
		{
			erased = true;
			if (boost::dynamic_pointer_cast<MifarePlusProfileSL3>(getChip()->getProfile())->getKeyUsage(i, KT_KEY_AES_B))
			{
				if (boost::dynamic_pointer_cast<MifarePlusCommandsSL3>(getCommands())->authenticate(i, profile->getKey(i, KT_KEY_AES_B), KT_KEY_AES_B))
				{
					for (j = ((i == 0) ? 1 : 0); j < getNbBlocks(i); ++j)
					{
						boost::dynamic_pointer_cast<MifarePlusCommandsSL3>(getCommands())->updateBinary(getBlockNo(i, j), false, true, zeroblock, MIFARE_PLUS_BLOCK_SIZE);
					}
				}
			}
			else if (boost::dynamic_pointer_cast<MifarePlusProfileSL3>(getChip()->getProfile())->getKeyUsage(i, KT_KEY_AES_A))
			{
				if (!erased && boost::dynamic_pointer_cast<MifarePlusCommandsSL3>(getCommands())->authenticate(i, profile->getKey(i, KT_KEY_AES_A), KT_KEY_AES_A))
				{
					for (j = ((i == 0) ? 1 : 0); j < getNbBlocks(i); ++j)
					{
						boost::dynamic_pointer_cast<MifarePlusCommandsSL3>(getCommands())->updateBinary(getBlockNo(i, j), false, true, zeroblock, MIFARE_PLUS_BLOCK_SIZE);
					}
				}
			}
			else
				success = false;
		}

		return success;
	}

	bool MifarePlusCardProviderSL3::erase(boost::shared_ptr<Location> location, boost::shared_ptr<AccessInfo> aiToUse)
	{
		unsigned char zeroblock[16];
		bool success = false;

		memset(zeroblock, 0x00, 16);

		if (!aiToUse || !location)
			return false;
		boost::shared_ptr<MifarePlusAccessInfo> mAiToUse = boost::dynamic_pointer_cast<MifarePlusAccessInfo>(aiToUse);
		boost::shared_ptr<MifarePlusLocation> mLocation = boost::dynamic_pointer_cast<MifarePlusLocation>(location);

		if (mAiToUse->keyB && !mAiToUse->keyB->isEmpty())
		{
			if (boost::dynamic_pointer_cast<MifarePlusCommandsSL3>(getCommands())->authenticate(mLocation->sector, mAiToUse->keyB, KT_KEY_AES_B))
			{
				boost::dynamic_pointer_cast<MifarePlusCommandsSL3>(getCommands())->updateBinary(getBlockNo(mLocation->sector, mLocation->block), false, true, zeroblock, MIFARE_PLUS_BLOCK_SIZE);
			}
		}
		else if (mAiToUse->keyA && !mAiToUse->keyA->isEmpty())
		{
			if (boost::dynamic_pointer_cast<MifarePlusCommandsSL3>(getCommands())->authenticate(mLocation->sector, mAiToUse->keyA, KT_KEY_AES_A))
			{
				boost::dynamic_pointer_cast<MifarePlusCommandsSL3>(getCommands())->updateBinary(getBlockNo(mLocation->sector, mLocation->block), false, true, zeroblock, MIFARE_PLUS_BLOCK_SIZE);
			}
		}
		else
			success = false;

		return success;
	}

	bool MifarePlusCardProviderSL3::writeData(boost::shared_ptr<Location> location, boost::shared_ptr<AccessInfo> /*aiToUse*/, boost::shared_ptr<AccessInfo> aiToWrite, const void* data, size_t dataLength, CardBehavior behaviorFlags)
	{
		int i;
		size_t bufIndex = 0;
		size_t toWriteLen = 0;
		size_t sectorLen;
		int stopSector;

		if (data == NULL || dataLength < MIFARE_PLUS_BLOCK_SIZE || dataLength % 16 != 0)
		{
			throw EXCEPTION(std::invalid_argument, "Bad buffer parameter. Data length must be multiple of 16.");
		}

		EXCEPTION_ASSERT(location, std::invalid_argument, "location cannot be null.");
		EXCEPTION_ASSERT(data, std::invalid_argument, "data cannot be null.");

		boost::shared_ptr<MifarePlusLocation> mLocation = boost::dynamic_pointer_cast<MifarePlusLocation>(location);
		boost::shared_ptr<MifarePlusAccessInfo> mAiToWrite = boost::dynamic_pointer_cast<MifarePlusAccessInfo>(aiToWrite);

		EXCEPTION_ASSERT(mLocation, std::invalid_argument, "location must be a MifarePlusLocation.");

		if (aiToWrite)
		{
			EXCEPTION_ASSERT(mAiToWrite, std::invalid_argument, "aiToUse must be a MifarePlusAccessInfo.");
		}
		else
		{
			mAiToWrite = boost::dynamic_pointer_cast<MifarePlusAccessInfo>(getChip()->getProfile()->createAccessInfo());
		}

		if (mLocation->block > 0)
		{
			sectorLen = (getNbBlocks(mLocation->sector) - mLocation->block) * MIFARE_PLUS_BLOCK_SIZE;
			if (dataLength < sectorLen)
				sectorLen = dataLength;
			if (mAiToWrite->keyB && !mAiToWrite->keyB->isEmpty())
				writeSector(mLocation->sector, mLocation->block, data, sectorLen, mAiToWrite->keyB, KT_KEY_AES_B);
			else if (mAiToWrite->keyA && !mAiToWrite->keyA->isEmpty())
				writeSector(mLocation->sector, mLocation->block, data, sectorLen, mAiToWrite->keyA, KT_KEY_AES_A);
			else
				throw EXCEPTION(std::invalid_argument, "You must set the writing key using the MifarePlusAccessInfo ");

			bufIndex += sectorLen;
			mLocation->sector += 1;
			mLocation->block = 0;
			if (!(behaviorFlags & CB_AUTOSWITCHAREA))
				return true;
		}

		if (bufIndex < dataLength)
		{
			if (!(behaviorFlags & CB_AUTOSWITCHAREA))
				stopSector = mLocation->sector;
			else
			{
				toWriteLen = 0;
				for (i = 0; bufIndex + toWriteLen < dataLength; ++i)
				{
					toWriteLen += getNbBlocks(i) * MIFARE_PLUS_BLOCK_SIZE;
				}
				stopSector = i;
			}
			writeSectors(mLocation->sector, stopSector, reinterpret_cast<const char*>(data) + bufIndex, dataLength - bufIndex, mAiToWrite);
		}

		return true;
	}

	bool MifarePlusCardProviderSL3::readData(boost::shared_ptr<Location> location, boost::shared_ptr<AccessInfo> aiToUse, void* data, size_t dataLength, CardBehavior behaviorFlags)
	{
		int i;
		size_t bufIndex = 0;
		size_t toReadLen = 0;
		size_t sectorLen;
		int stopSector;

		if (data == NULL || dataLength < MIFARE_PLUS_BLOCK_SIZE || dataLength % 16 != 0)
		{
			throw EXCEPTION(std::invalid_argument, "Bad buffer parameter. Data length must be multiple of 16.");
		}

		EXCEPTION_ASSERT(location, std::invalid_argument, "location cannot be null.");
		EXCEPTION_ASSERT(data, std::invalid_argument, "data cannot be null.");

		boost::shared_ptr<MifarePlusLocation> mLocation = boost::dynamic_pointer_cast<MifarePlusLocation>(location);
		boost::shared_ptr<MifarePlusAccessInfo> mAiToUse = boost::dynamic_pointer_cast<MifarePlusAccessInfo>(aiToUse);

		EXCEPTION_ASSERT(mLocation, std::invalid_argument, "location must be a MifarePlusLocation.");

		if (aiToUse)
		{
			EXCEPTION_ASSERT(mAiToUse, std::invalid_argument, "aiToUse must be a MifarePlusAccessInfo.");
		}
		else
		{
			mAiToUse = boost::dynamic_pointer_cast<MifarePlusAccessInfo>(getChip()->getProfile()->createAccessInfo());
		}

		if (mLocation->block > 0)
		{
			sectorLen = (getNbBlocks(mLocation->sector) - mLocation->block) * MIFARE_PLUS_BLOCK_SIZE;
			if (dataLength < sectorLen)
				sectorLen = dataLength;
			if (mAiToUse->keyA && !mAiToUse->keyA->isEmpty())
				readSector(mLocation->sector, mLocation->block, data, sectorLen, mAiToUse->keyA, KT_KEY_AES_A);
			else if (mAiToUse->keyB && !mAiToUse->keyB->isEmpty())
				readSector(mLocation->sector, mLocation->block, data, sectorLen, mAiToUse->keyB, KT_KEY_AES_B);
			else
				throw EXCEPTION(std::invalid_argument, "You must set the read key using the MifarePlusAccessInfo ");

			bufIndex += sectorLen;
			mLocation->sector += 1;
			mLocation->block = 0;
			if (!(behaviorFlags & CB_AUTOSWITCHAREA))
				return true;
		}

		if (bufIndex < dataLength)
		{
			if (!(behaviorFlags & CB_AUTOSWITCHAREA))
				stopSector = mLocation->sector;
			else
			{
				toReadLen = 0;
				for (i = 0; bufIndex + toReadLen < dataLength; ++i)
				{
					toReadLen += getNbBlocks(i) * MIFARE_PLUS_BLOCK_SIZE;
				}
				stopSector = i;
			}
			readSectors(mLocation->sector, stopSector, reinterpret_cast<char*>(data) + bufIndex, dataLength - bufIndex, mAiToUse);
		}

		return true;
	}

	size_t MifarePlusCardProviderSL3::readDataHeader(boost::shared_ptr<Location> /*location*/, boost::shared_ptr<AccessInfo> /*aiToUse*/, void* /*data*/, size_t /*dataLength*/)
	{
		return 0;
	}

	void MifarePlusCardProviderSL3::writeSector(int sector, int start_block, const void* buf, size_t buflen, boost::shared_ptr<MifarePlusKey> key, MifarePlusKeyType keytype)
	{
		if (buf == NULL || buflen < MIFARE_PLUS_BLOCK_SIZE || buflen % MIFARE_PLUS_BLOCK_SIZE != 0 || buflen / MIFARE_PLUS_BLOCK_SIZE + start_block > getNbBlocks(sector))
		{
			throw EXCEPTION(std::invalid_argument, "Bad buffer parameter. The minimum buffer's length is 16 bytes.");
		}

		if (boost::dynamic_pointer_cast<MifarePlusCommandsSL3>(getCommands())->authenticate(sector, key, keytype))
		{
			boost::dynamic_pointer_cast<MifarePlusCommandsSL3>(getCommands())->updateBinary(getBlockNo(sector, start_block), false, true, buf, buflen);
		}
	}

	void MifarePlusCardProviderSL3::writeSectors(int start_sector, int stop_sector, const void* buf, size_t buflen, boost::shared_ptr<AccessInfo> aiToUse)
	{
		int i;
		size_t test_buflen = 0;
		size_t toWriteLen = 0;

		if (aiToUse)
		{
			boost::shared_ptr<MifarePlusAccessInfo> mAiToUse = boost::dynamic_pointer_cast<MifarePlusAccessInfo>(aiToUse);
			EXCEPTION_ASSERT(mAiToUse, std::invalid_argument, "aiToUse must be a MifarePlusAccessInfo.");

			for (i = start_sector; i < stop_sector; ++i)
			{
				test_buflen += getNbBlocks(i) * MIFARE_PLUS_BLOCK_SIZE;
			}
			if (test_buflen + MIFARE_PLUS_BLOCK_SIZE != buflen)
				throw EXCEPTION(std::invalid_argument, "Bad buffer parameter. The buffer must fit the sectors size.");

			test_buflen = 0;
			for (i = start_sector; i <= stop_sector && test_buflen < buflen; ++i)
			{
				toWriteLen = getNbBlocks(i) * MIFARE_PLUS_BLOCK_SIZE;
				if (toWriteLen + test_buflen > buflen)
					toWriteLen = buflen - test_buflen;
				if (mAiToUse->keyB && !mAiToUse->keyB->isEmpty())
					writeSector(i, 0, reinterpret_cast<const char*>(buf) + test_buflen, toWriteLen, mAiToUse->keyB, KT_KEY_AES_B);
				else if (mAiToUse->keyA && !mAiToUse->keyA->isEmpty())
					writeSector(i, 0, reinterpret_cast<const char*>(buf) + test_buflen, toWriteLen, mAiToUse->keyA, KT_KEY_AES_A);
				else
					throw EXCEPTION(std::invalid_argument, "You must set the writing key using the MifarePlusAccessInfo ");
				test_buflen += getNbBlocks(i) * MIFARE_PLUS_BLOCK_SIZE;
			}
		}
	}

	void MifarePlusCardProviderSL3::readSector(int sector, int start_block, void* buf, size_t buflen, boost::shared_ptr<MifarePlusKey> key, MifarePlusKeyType keytype)
	{
		if (buf == NULL || buflen < MIFARE_PLUS_BLOCK_SIZE || buflen % MIFARE_PLUS_BLOCK_SIZE != 0 || buflen / MIFARE_PLUS_BLOCK_SIZE + start_block > getNbBlocks(sector))
		{
			throw EXCEPTION(std::invalid_argument, "Bad buffer parameter. The minimum buffer's length is 16 bytes.");
		}

		if (boost::dynamic_pointer_cast<MifarePlusCommandsSL3>(getCommands())->authenticate(sector, key, keytype))
		{
			boost::dynamic_pointer_cast<MifarePlusCommandsSL3>(getCommands())->readBinary(getBlockNo(sector, start_block), static_cast<unsigned char>(buflen / MIFARE_PLUS_BLOCK_SIZE), false, true, true, buf, buflen);
		}
	}

	void MifarePlusCardProviderSL3::readSectors(int start_sector, int stop_sector, void* buf, size_t buflen, boost::shared_ptr<AccessInfo> aiToUse)
	{
		int i;
		size_t test_buflen = 0;
		size_t toReadLen = 0;

		if (aiToUse)
		{
			boost::shared_ptr<MifarePlusAccessInfo> mAiToUse = boost::dynamic_pointer_cast<MifarePlusAccessInfo>(aiToUse);
			EXCEPTION_ASSERT(mAiToUse, std::invalid_argument, "aiToUse must be a MifarePlusAccessInfo.");

			for (i = start_sector; i < stop_sector; ++i)
			{
				test_buflen += getNbBlocks(i) * MIFARE_PLUS_BLOCK_SIZE;
			}
			if (test_buflen + MIFARE_PLUS_BLOCK_SIZE > buflen)
				throw EXCEPTION(std::invalid_argument, "Bad buffer parameter. The buffer must fit the sectors size.");

			test_buflen = 0;
			for (i = start_sector; i <= stop_sector && test_buflen < buflen; ++i)
			{
				toReadLen = getNbBlocks(i) * MIFARE_PLUS_BLOCK_SIZE;
				if (toReadLen + test_buflen > buflen)
					toReadLen = buflen - test_buflen;
				if (mAiToUse->keyA && !mAiToUse->keyA->isEmpty())
					readSector(i, 0, reinterpret_cast<char*>(buf) + test_buflen, toReadLen, mAiToUse->keyA, KT_KEY_AES_A);
				else if (mAiToUse->keyB && !mAiToUse->keyB->isEmpty())
					readSector(i, 0, reinterpret_cast<char*>(buf) + test_buflen, toReadLen, mAiToUse->keyB, KT_KEY_AES_B);
				else
					throw EXCEPTION(std::invalid_argument, "You must set the writing key using the MifarePlusAccessInfo ");
				test_buflen += getNbBlocks(i) * MIFARE_PLUS_BLOCK_SIZE;
			}
		}
	}

	bool MifarePlusCardProviderSL3::authenticate(boost::shared_ptr<Location> location, boost::shared_ptr<AccessInfo> ai)
	{
		boost::shared_ptr<MifarePlusAccessInfo> mAiToUse = boost::dynamic_pointer_cast<MifarePlusAccessInfo>(ai);

		if (ai)
		{
			EXCEPTION_ASSERT(mAiToUse, std::invalid_argument, "ai must be a MifarePlusAccessInfo.");
		}
		else
		{
			mAiToUse = boost::dynamic_pointer_cast<MifarePlusAccessInfo>(getChip()->getProfile()->createAccessInfo());
		}

		if (mAiToUse->keyConfiguration && !mAiToUse->keyConfiguration->isEmpty())
			boost::dynamic_pointer_cast<MifarePlusCommandsSL3>(getCommands())->authenticate(0, mAiToUse->keyConfiguration, KT_KEY_CONFIGURATION);
		else if (mAiToUse->keyMastercard && !mAiToUse->keyMastercard->isEmpty())
			boost::dynamic_pointer_cast<MifarePlusCommandsSL3>(getCommands())->authenticate(0, mAiToUse->keyMastercard, KT_KEY_MASTERCARD);
		if (mAiToUse->keyOriginality && !mAiToUse->keyOriginality->isEmpty())
			boost::dynamic_pointer_cast<MifarePlusCommandsSL3>(getCommands())->authenticate(0, mAiToUse->keyOriginality, KT_KEY_ORIGINALITY);
		if (mAiToUse->keyA && !mAiToUse->keyA->isEmpty())
		{
			EXCEPTION_ASSERT(location, std::invalid_argument, "location cannot be null.");
			boost::shared_ptr<MifarePlusLocation> mLocation = boost::dynamic_pointer_cast<MifarePlusLocation>(location);
			EXCEPTION_ASSERT(mLocation, std::invalid_argument, "location must be a MifarePlusLocation.");
			
			boost::dynamic_pointer_cast<MifarePlusCommandsSL3>(getCommands())->authenticate(mLocation->sector, mAiToUse->keyA, KT_KEY_AES_A);
		}
		if (mAiToUse->keyB && !mAiToUse->keyB->isEmpty())
		{
			EXCEPTION_ASSERT(location, std::invalid_argument, "location cannot be null.");
			boost::shared_ptr<MifarePlusLocation> mLocation = boost::dynamic_pointer_cast<MifarePlusLocation>(location);
			EXCEPTION_ASSERT(mLocation, std::invalid_argument, "location must be a MifarePlusLocation.");
			
			boost::dynamic_pointer_cast<MifarePlusCommandsSL3>(getCommands())->authenticate(mLocation->sector, mAiToUse->keyB, KT_KEY_AES_B);
		}

		return true;
	}

	unsigned char MifarePlusCardProviderSL3::getNbBlocks(int sector) const
	{
		return ((sector >= 32) ? 15 : 3);
	}

	unsigned short MifarePlusCardProviderSL3::getBlockNo(int sector, int block)
	{
		unsigned short blockno = 0x0000;
		int i;

		for (i = 0; i < sector; ++i)
		{
			blockno += getNbBlocks(i) + 1;
		}
		blockno += static_cast<unsigned short>(block);

		return blockno;
	}
}

