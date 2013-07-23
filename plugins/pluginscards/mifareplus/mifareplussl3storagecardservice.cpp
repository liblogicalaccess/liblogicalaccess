/**
 * \file mifareplussl3storagecardservice.cpp
 * \author Xavier SCHMERBER <xavier.schmerber@gmail.com>
 * \brief MifarePlus SL3 storage card service.
 */

#include "mifareplussl3storagecardservice.hpp"
#include "mifarepluschip.hpp"
#include "mifarepluskey.hpp"


#include <iostream>
#include <iomanip>
#include <sstream>
#include <cstring>

namespace logicalaccess
{	
	MifarePlusSL3StorageCardService::MifarePlusSL3StorageCardService(boost::shared_ptr<Chip> chip)
		: StorageCardService(chip)
	{
	}

	MifarePlusSL3StorageCardService::~MifarePlusSL3StorageCardService()
	{
	}

	bool MifarePlusSL3StorageCardService::erase()
	{
		boost::shared_ptr<MifarePlusSL3Profile> profile = boost::dynamic_pointer_cast<MifarePlusSL3Profile>(getChip()->getProfile());
		unsigned int i;
		unsigned int j;
		unsigned char zeroblock[16];
		bool erased;
		bool success = true;

		memset(zeroblock, 0x00, 16);

		for (i = 0; i < boost::dynamic_pointer_cast<MifarePlusChip>(getChip())->getNbSectors(); ++i)
		{
			erased = true;
			if (boost::dynamic_pointer_cast<MifarePlusSL3Profile>(getChip()->getProfile())->getKeyUsage(i, KT_KEY_AES_B))
			{
				if (getMifarePlusChip()->getMifarePlusSL3Commands()->authenticate(i, profile->getKey(i, KT_KEY_AES_B), KT_KEY_AES_B))
				{
					for (j = ((i == 0) ? 1 : 0); j < getMifarePlusChip()->getMifarePlusSL3Commands()->getNbBlocks(i); ++j)
					{
						getMifarePlusChip()->getMifarePlusSL3Commands()->updateBinary(getMifarePlusChip()->getMifarePlusSL3Commands()->getBlockNo(i, j), false, true, zeroblock, MIFARE_PLUS_BLOCK_SIZE);
					}
				}
			}
			else if (boost::dynamic_pointer_cast<MifarePlusSL3Profile>(getChip()->getProfile())->getKeyUsage(i, KT_KEY_AES_A))
			{
				if (!erased && getMifarePlusChip()->getMifarePlusSL3Commands()->authenticate(i, profile->getKey(i, KT_KEY_AES_A), KT_KEY_AES_A))
				{
					for (j = ((i == 0) ? 1 : 0); j < getMifarePlusChip()->getMifarePlusSL3Commands()->getNbBlocks(i); ++j)
					{
						getMifarePlusChip()->getMifarePlusSL3Commands()->updateBinary(getMifarePlusChip()->getMifarePlusSL3Commands()->getBlockNo(i, j), false, true, zeroblock, MIFARE_PLUS_BLOCK_SIZE);
					}
				}
			}
			else
				success = false;
		}

		return success;
	}

	bool MifarePlusSL3StorageCardService::erase(boost::shared_ptr<Location> location, boost::shared_ptr<AccessInfo> aiToUse)
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
			if (getMifarePlusChip()->getMifarePlusSL3Commands()->authenticate(mLocation->sector, mAiToUse->keyB, KT_KEY_AES_B))
			{
				getMifarePlusChip()->getMifarePlusSL3Commands()->updateBinary(getMifarePlusChip()->getMifarePlusSL3Commands()->getBlockNo(mLocation->sector, mLocation->block), false, true, zeroblock, MIFARE_PLUS_BLOCK_SIZE);
			}
		}
		else if (mAiToUse->keyA && !mAiToUse->keyA->isEmpty())
		{
			if (getMifarePlusChip()->getMifarePlusSL3Commands()->authenticate(mLocation->sector, mAiToUse->keyA, KT_KEY_AES_A))
			{
				getMifarePlusChip()->getMifarePlusSL3Commands()->updateBinary(getMifarePlusChip()->getMifarePlusSL3Commands()->getBlockNo(mLocation->sector, mLocation->block), false, true, zeroblock, MIFARE_PLUS_BLOCK_SIZE);
			}
		}
		else
			success = false;

		return success;
	}

	bool MifarePlusSL3StorageCardService::writeData(boost::shared_ptr<Location> location, boost::shared_ptr<AccessInfo> /*aiToUse*/, boost::shared_ptr<AccessInfo> aiToWrite, const void* data, size_t dataLength, CardBehavior behaviorFlags)
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
			sectorLen = (getMifarePlusChip()->getMifarePlusSL3Commands()->getNbBlocks(mLocation->sector) - mLocation->block) * MIFARE_PLUS_BLOCK_SIZE;
			if (dataLength < sectorLen)
				sectorLen = dataLength;
			if (mAiToWrite->keyB && !mAiToWrite->keyB->isEmpty())
				getMifarePlusChip()->getMifarePlusSL3Commands()->writeSector(mLocation->sector, mLocation->block, data, sectorLen, mAiToWrite->keyB, KT_KEY_AES_B);
			else if (mAiToWrite->keyA && !mAiToWrite->keyA->isEmpty())
				getMifarePlusChip()->getMifarePlusSL3Commands()->writeSector(mLocation->sector, mLocation->block, data, sectorLen, mAiToWrite->keyA, KT_KEY_AES_A);
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
					toWriteLen += getMifarePlusChip()->getMifarePlusSL3Commands()->getNbBlocks(i) * MIFARE_PLUS_BLOCK_SIZE;
				}
				stopSector = i;
			}
			getMifarePlusChip()->getMifarePlusSL3Commands()->writeSectors(mLocation->sector, stopSector, reinterpret_cast<const char*>(data) + bufIndex, dataLength - bufIndex, mAiToWrite);
		}

		return true;
	}

	bool MifarePlusSL3StorageCardService::readData(boost::shared_ptr<Location> location, boost::shared_ptr<AccessInfo> aiToUse, void* data, size_t dataLength, CardBehavior behaviorFlags)
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
			sectorLen = (getMifarePlusChip()->getMifarePlusSL3Commands()->getNbBlocks(mLocation->sector) - mLocation->block) * MIFARE_PLUS_BLOCK_SIZE;
			if (dataLength < sectorLen)
				sectorLen = dataLength;
			if (mAiToUse->keyA && !mAiToUse->keyA->isEmpty())
				getMifarePlusChip()->getMifarePlusSL3Commands()->readSector(mLocation->sector, mLocation->block, data, sectorLen, mAiToUse->keyA, KT_KEY_AES_A);
			else if (mAiToUse->keyB && !mAiToUse->keyB->isEmpty())
				getMifarePlusChip()->getMifarePlusSL3Commands()->readSector(mLocation->sector, mLocation->block, data, sectorLen, mAiToUse->keyB, KT_KEY_AES_B);
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
					toReadLen += getMifarePlusChip()->getMifarePlusSL3Commands()->getNbBlocks(i) * MIFARE_PLUS_BLOCK_SIZE;
				}
				stopSector = i;
			}
			getMifarePlusChip()->getMifarePlusSL3Commands()->readSectors(mLocation->sector, stopSector, reinterpret_cast<char*>(data) + bufIndex, dataLength - bufIndex, mAiToUse);
		}

		return true;
	}

	size_t MifarePlusSL3StorageCardService::readDataHeader(boost::shared_ptr<Location> /*location*/, boost::shared_ptr<AccessInfo> /*aiToUse*/, void* /*data*/, size_t /*dataLength*/)
	{
		return 0;
	}
}

