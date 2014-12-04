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
    MifarePlusSL3StorageCardService::MifarePlusSL3StorageCardService(std::shared_ptr<Chip> chip)
        : StorageCardService(chip)
    {
    }

    MifarePlusSL3StorageCardService::~MifarePlusSL3StorageCardService()
    {
    }

    void MifarePlusSL3StorageCardService::erase()
    {
        std::shared_ptr<MifarePlusSL3Profile> profile = std::dynamic_pointer_cast<MifarePlusSL3Profile>(getChip()->getProfile());
        unsigned int i;
        unsigned int j;
        std::vector<unsigned char> zeroblock(16, 0x00);

        for (i = 0; i < std::dynamic_pointer_cast<MifarePlusChip>(getChip())->getNbSectors(); ++i)
        {
            if (std::dynamic_pointer_cast<MifarePlusSL3Profile>(getChip()->getProfile())->getKeyUsage(i, KT_KEY_AES_B))
            {
                if (getMifarePlusChip()->getMifarePlusSL3Commands()->authenticate(i, profile->getKey(i, KT_KEY_AES_B), KT_KEY_AES_B))
                {
                    for (j = ((i == 0) ? 1 : 0); j < getMifarePlusChip()->getMifarePlusSL3Commands()->getNbBlocks(i); ++j)
                    {
                        getMifarePlusChip()->getMifarePlusSL3Commands()->updateBinary(getMifarePlusChip()->getMifarePlusSL3Commands()->getBlockNo(i, j), false, true, zeroblock);
                    }
                }
            }
        }
    }

    void MifarePlusSL3StorageCardService::erase(std::shared_ptr<Location> location, std::shared_ptr<AccessInfo> aiToUse)
    {
        std::vector<unsigned char> zeroblock(16, 0x00);

        if (!aiToUse || !location)
            return;
        std::shared_ptr<MifarePlusAccessInfo> mAiToUse = std::dynamic_pointer_cast<MifarePlusAccessInfo>(aiToUse);
        std::shared_ptr<MifarePlusLocation> mLocation = std::dynamic_pointer_cast<MifarePlusLocation>(location);

        if (mAiToUse->keyB && !mAiToUse->keyB->isEmpty())
        {
            if (getMifarePlusChip()->getMifarePlusSL3Commands()->authenticate(mLocation->sector, mAiToUse->keyB, KT_KEY_AES_B))
            {
                getMifarePlusChip()->getMifarePlusSL3Commands()->updateBinary(getMifarePlusChip()->getMifarePlusSL3Commands()->getBlockNo(mLocation->sector, mLocation->block), false, true, zeroblock);
            }
        }
        else if (mAiToUse->keyA && !mAiToUse->keyA->isEmpty())
        {
            if (getMifarePlusChip()->getMifarePlusSL3Commands()->authenticate(mLocation->sector, mAiToUse->keyA, KT_KEY_AES_A))
            {
                getMifarePlusChip()->getMifarePlusSL3Commands()->updateBinary(getMifarePlusChip()->getMifarePlusSL3Commands()->getBlockNo(mLocation->sector, mLocation->block), false, true, zeroblock);
            }
        }
    }

    void MifarePlusSL3StorageCardService::writeData(std::shared_ptr<Location> location, std::shared_ptr<AccessInfo> /*aiToUse*/, std::shared_ptr<AccessInfo> aiToWrite, const std::vector<unsigned char>& data, CardBehavior behaviorFlags)
    {
        if (data.size() < MIFARE_PLUS_BLOCK_SIZE || data.size() % 16 != 0)
        {
            THROW_EXCEPTION_WITH_LOG(std::invalid_argument, "Bad buffer parameter. Data length must be multiple of 16.");
        }

        EXCEPTION_ASSERT(location, std::invalid_argument, "location cannot be null.");

        std::shared_ptr<MifarePlusLocation> mLocation = std::dynamic_pointer_cast<MifarePlusLocation>(location);
        std::shared_ptr<MifarePlusAccessInfo> mAiToWrite = std::dynamic_pointer_cast<MifarePlusAccessInfo>(aiToWrite);

        EXCEPTION_ASSERT(mLocation, std::invalid_argument, "location must be a MifarePlusLocation.");

        if (aiToWrite)
        {
            EXCEPTION_ASSERT(mAiToWrite, std::invalid_argument, "aiToUse must be a MifarePlusAccessInfo.");
        }
        else
        {
            mAiToWrite = std::dynamic_pointer_cast<MifarePlusAccessInfo>(getChip()->getProfile()->createAccessInfo());
        }

        size_t bufIndex = 0;
        if (mLocation->block > 0)
        {
            size_t sectorLen = (getMifarePlusChip()->getMifarePlusSL3Commands()->getNbBlocks(mLocation->sector) - mLocation->block) * MIFARE_PLUS_BLOCK_SIZE;
            if (data.size() < sectorLen)
                sectorLen = data.size();
			std::vector<unsigned char> tmp(data.begin(), data.begin() + sectorLen);
            if (mAiToWrite->keyB && !mAiToWrite->keyB->isEmpty())
                getMifarePlusChip()->getMifarePlusSL3Commands()->writeSector(mLocation->sector, mLocation->block, tmp, mAiToWrite->keyB, KT_KEY_AES_B);
            else if (mAiToWrite->keyA && !mAiToWrite->keyA->isEmpty())
                getMifarePlusChip()->getMifarePlusSL3Commands()->writeSector(mLocation->sector, mLocation->block, tmp, mAiToWrite->keyA, KT_KEY_AES_A);
            else
                THROW_EXCEPTION_WITH_LOG(std::invalid_argument, "You must set the writing key using the MifarePlusAccessInfo ");

            bufIndex += sectorLen;
            mLocation->sector += 1;
            mLocation->block = 0;
            if (!(behaviorFlags & CB_AUTOSWITCHAREA))
                return;
        }

        if (bufIndex < data.size())
        {
            int stopSector;
            if (!(behaviorFlags & CB_AUTOSWITCHAREA))
                stopSector = mLocation->sector;
            else
            {
                size_t toWriteLen = 0;
                int i;
                for (i = 0; bufIndex + toWriteLen < data.size(); ++i)
                {
                    toWriteLen += getMifarePlusChip()->getMifarePlusSL3Commands()->getNbBlocks(i) * MIFARE_PLUS_BLOCK_SIZE;
                }
                stopSector = i;
            }
			std::vector<unsigned char> tmp(data.begin() + bufIndex, data.end());
            getMifarePlusChip()->getMifarePlusSL3Commands()->writeSectors(mLocation->sector, stopSector, tmp, mAiToWrite);
        }
    }

    std::vector<unsigned char> MifarePlusSL3StorageCardService::readData(std::shared_ptr<Location> location, std::shared_ptr<AccessInfo> aiToUse, size_t length, CardBehavior behaviorFlags)
    {
		std::vector<unsigned char> ret, data;
        EXCEPTION_ASSERT(location, std::invalid_argument, "location cannot be null.");

        std::shared_ptr<MifarePlusLocation> mLocation = std::dynamic_pointer_cast<MifarePlusLocation>(location);
        std::shared_ptr<MifarePlusAccessInfo> mAiToUse = std::dynamic_pointer_cast<MifarePlusAccessInfo>(aiToUse);

        EXCEPTION_ASSERT(mLocation, std::invalid_argument, "location must be a MifarePlusLocation.");

        if (aiToUse)
        {
            EXCEPTION_ASSERT(mAiToUse, std::invalid_argument, "aiToUse must be a MifarePlusAccessInfo.");
        }
        else
        {
            mAiToUse = std::dynamic_pointer_cast<MifarePlusAccessInfo>(getChip()->getProfile()->createAccessInfo());
        }

        size_t bufIndex = 0;
        if (mLocation->block > 0)
        {
            size_t sectorLen = (getMifarePlusChip()->getMifarePlusSL3Commands()->getNbBlocks(mLocation->sector) - mLocation->block) * MIFARE_PLUS_BLOCK_SIZE;
            if (length < sectorLen)
                sectorLen = length;

            if (mAiToUse->keyA && !mAiToUse->keyA->isEmpty())
                data = getMifarePlusChip()->getMifarePlusSL3Commands()->readSector(mLocation->sector, mLocation->block, mAiToUse->keyA, KT_KEY_AES_A);
            else if (mAiToUse->keyB && !mAiToUse->keyB->isEmpty())
                data = getMifarePlusChip()->getMifarePlusSL3Commands()->readSector(mLocation->sector, mLocation->block, mAiToUse->keyB, KT_KEY_AES_B);
            else
                THROW_EXCEPTION_WITH_LOG(std::invalid_argument, "You must set the read key using the MifarePlusAccessInfo ");

			ret.insert(ret.end(), data.begin(), data.end());
            bufIndex = data.size();
            mLocation->sector += 1;
            mLocation->block = 0;
            if (!(behaviorFlags & CB_AUTOSWITCHAREA))
                return std::vector<unsigned char>();
        }

        if (bufIndex < length)
        {
            int stopSector;
            if (!(behaviorFlags & CB_AUTOSWITCHAREA))
                stopSector = mLocation->sector;
            else
            {
                size_t toReadLen = 0;
                int i;
                for (i = 0; bufIndex + toReadLen < length; ++i)
                {
                    toReadLen += getMifarePlusChip()->getMifarePlusSL3Commands()->getNbBlocks(i) * MIFARE_PLUS_BLOCK_SIZE;
                }
                stopSector = i;
            }
            data = getMifarePlusChip()->getMifarePlusSL3Commands()->readSectors(mLocation->sector, stopSector, mAiToUse);
			ret.insert(ret.end(), data.begin(), data.end());
        }
		return ret;
    }

    unsigned int MifarePlusSL3StorageCardService::readDataHeader(std::shared_ptr<Location> /*location*/, std::shared_ptr<AccessInfo> /*aiToUse*/, void* /*data*/, size_t /*dataLength*/)
    {
        return 0;
    }
}