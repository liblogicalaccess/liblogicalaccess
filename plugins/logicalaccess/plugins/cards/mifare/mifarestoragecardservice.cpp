/**
 * \file mifarestoragecardservice.cpp
 * \author Julien K. <julien-dev@islog.com>, Maxime C. <maxime-dev@islog.com>
 * \brief Mifare storage card service.
 */

#include <cstring>
#include <logicalaccess/plugins/llacommon/logs.hpp>
#include <assert.h>
#include <logicalaccess/plugins/cards/mifare/mifarestoragecardservice.hpp>
#include <logicalaccess/plugins/cards/mifare/mifarechip.hpp>

namespace logicalaccess
{
MifareStorageCardService::MifareStorageCardService(std::shared_ptr<Chip> chip)
    : StorageCardService(chip)
{
}

MifareStorageCardService::~MifareStorageCardService()
{
}

void MifareStorageCardService::erase(std::shared_ptr<Location> location,
                                     std::shared_ptr<AccessInfo> aiToUse)
{
    std::shared_ptr<MifareLocation> mLocation =
        std::dynamic_pointer_cast<MifareLocation>(location);
    if (!mLocation)
    {
        return;
    }

    unsigned int zeroblock_size =
        getMifareChip()->getMifareCommands()->getNbBlocks(mLocation->sector) * 16;

    std::shared_ptr<MifareAccessInfo> _aiToWrite;
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
        std::shared_ptr<MifareLocation> madLocation(new MifareLocation());
        madLocation->sector = 0;
        madLocation->block  = 1;

        std::shared_ptr<MifareAccessInfo> madAi(new MifareAccessInfo());
        if (aiToUse)
        {
            std::shared_ptr<MifareAccessInfo> mAiToUse =
                std::dynamic_pointer_cast<MifareAccessInfo>(aiToUse);
            if (mAiToUse->useMAD)
            {
                madAi->keyA = mAiToUse->madKeyA;
                madAi->keyB = mAiToUse->madKeyB;
            }
        }

        if (madAi->keyB && !madAi->keyB->isEmpty())
        {
            madAi->sab.setAReadBWriteConfiguration();
        }
        ByteVector zeroblock(32, 0x00);
        writeData(madLocation, madAi, _aiToWrite, zeroblock, CB_DEFAULT);
    }

    if (mLocation->block == -1)
    {
        mLocation->block = 0;
    }

    bool tmpuseMAD    = mLocation->useMAD;
    mLocation->useMAD = false;
    ByteVector zeroblock(zeroblock_size - (mLocation->block * 16), 0x00);
    writeData(location, aiToUse, _aiToWrite, zeroblock, CB_DEFAULT);
    mLocation->useMAD = tmpuseMAD;
}

void MifareStorageCardService::writeData(std::shared_ptr<Location> location,
                                         std::shared_ptr<AccessInfo> aiToUse,
                                         std::shared_ptr<AccessInfo> aiToWrite,
                                         const ByteVector &data,
                                         CardBehavior behaviorFlags)
{
    EXCEPTION_ASSERT_WITH_LOG(location, std::invalid_argument,
                              "location cannot be null.");

    std::shared_ptr<MifareLocation> mLocation =
        std::dynamic_pointer_cast<MifareLocation>(location);
    std::shared_ptr<MifareAccessInfo> mAiToUse =
        std::dynamic_pointer_cast<MifareAccessInfo>(aiToUse);

    EXCEPTION_ASSERT_WITH_LOG(mLocation, std::invalid_argument,
                              "location must be a MifareLocation.");

    if (aiToUse)
    {
        EXCEPTION_ASSERT_WITH_LOG(mAiToUse, std::invalid_argument,
                                  "aiToUse must be a MifareAccessInfo.");
    }
    else
    {
        mAiToUse =
            std::dynamic_pointer_cast<MifareAccessInfo>(getChip()->createAccessInfo());
    }

    bool writeAidToMad = false;

    if (mLocation->useMAD)
    {
        if (mLocation->sector == 0)
        {
            mLocation->sector = getMifareChip()->getMifareCommands()->getSectorFromMAD(
                mLocation->aid, mAiToUse->madKeyA);
        }
        else
        {
            writeAidToMad = true;
        }
    }

    if (mLocation->block == -1)
    {
        mLocation->block = 0;
    }

    size_t totaldatalen = data.size() + (mLocation->block * 16) + mLocation->byte_;
    int nbSectors       = 0;
    size_t totalbuflen  = 0;
    while (totalbuflen < totaldatalen)
    {
        totalbuflen += getMifareChip()->getMifareCommands()->getNbBlocks(
                           mLocation->sector + nbSectors) *
                       16;
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
        ByteVector dataSectors;
        dataSectors.resize(buflen, 0x00);
        std::copy(data.begin(), data.end(), dataSectors.begin() + mLocation->byte_);

        if (writeAidToMad)
        {
            for (int i = mLocation->sector; i < (mLocation->sector + nbSectors); ++i)
            {
                getMifareChip()->getMifareCommands()->setSectorToMAD(
                    mLocation->aid, i, mAiToUse->madKeyA, mAiToUse->madKeyB);
            }
        }

        std::shared_ptr<MifareAccessInfo> mAiToWrite;
        // Write access informations too
        if (aiToWrite)
        {
            // Sector keys will be changed after writing
            mAiToWrite = std::dynamic_pointer_cast<MifareAccessInfo>(aiToWrite);

            EXCEPTION_ASSERT_WITH_LOG(mAiToWrite, std::invalid_argument,
                                      "mAiToWrite must be a MifareAccessInfo.");

            // MAD keys are changed now
            if (writeAidToMad && mAiToWrite->useMAD)
            {
                unsigned int madsector            = (mLocation->sector <= 16) ? 0 : 16;
                MifareKeyType madKeyType          = KT_KEY_A;
                std::shared_ptr<MifareKey> madKey = mAiToUse->madKeyA;

                if (!mAiToUse->madKeyB->isEmpty())
                {
                    madKeyType = KT_KEY_B;
                    madKey     = mAiToUse->madKeyB;
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

                getMifareChip()->getMifareCommands()->changeKeys(
                    madKeyType, madKey, mAiToWrite->madKeyA, mAiToWrite->madKeyB,
                    madsector, &newsab, mAiToWrite->madGPB);
            }

            // No key change, set to null (to not change sab, ...)
            if (mAiToWrite->keyA->isEmpty() && mAiToWrite->keyB->isEmpty())
            {
                mAiToWrite.reset();
            }
        }

        if (behaviorFlags & CB_AUTOSWITCHAREA)
        {
            getMifareChip()->getMifareCommands()->writeSectors(
                mLocation->sector, mLocation->sector + nbSectors - 1, mLocation->block,
                dataSectors, mAiToUse->keyA, mAiToUse->keyB, mAiToUse->sab,
                mAiToWrite ? mAiToWrite->gpb : mAiToUse->gpb,
                mAiToWrite ? &(mAiToWrite->sab) : NULL,
                mAiToWrite ? mAiToWrite->keyA : std::shared_ptr<MifareKey>(),
                mAiToWrite ? mAiToWrite->keyB : std::shared_ptr<MifareKey>());
        }
        else
        {
            getMifareChip()->getMifareCommands()->writeSector(
                mLocation->sector, mLocation->block, dataSectors, mAiToUse->keyA,
                mAiToUse->keyB, mAiToUse->sab,
                mAiToWrite ? mAiToWrite->gpb : mAiToUse->gpb,
                mAiToWrite ? &(mAiToWrite->sab) : NULL,
                mAiToWrite ? mAiToWrite->keyA : std::shared_ptr<MifareKey>(),
                mAiToWrite ? mAiToWrite->keyB : std::shared_ptr<MifareKey>());
        }
    }
}

ByteVector MifareStorageCardService::readData(std::shared_ptr<Location> location,
                                              std::shared_ptr<AccessInfo> aiToUse,
                                              size_t length, CardBehavior behaviorFlags)
{
    ByteVector ret;
    EXCEPTION_ASSERT_WITH_LOG(location, std::invalid_argument,
                              "location cannot be null.");

    std::shared_ptr<MifareLocation> mLocation =
        std::dynamic_pointer_cast<MifareLocation>(location);
    std::shared_ptr<MifareAccessInfo> mAiToUse =
        std::dynamic_pointer_cast<MifareAccessInfo>(aiToUse);

    EXCEPTION_ASSERT_WITH_LOG(mLocation, std::invalid_argument,
                              "location must be a MifareLocation.");
    if (aiToUse)
    {
        EXCEPTION_ASSERT_WITH_LOG(mAiToUse, std::invalid_argument,
                                  "aiToUse must be a MifareAccessInfo.");
    }
    else
    {
        mAiToUse =
            std::dynamic_pointer_cast<MifareAccessInfo>(getChip()->createAccessInfo());
    }

    if (mLocation->useMAD)
    {
        mLocation->sector = getMifareChip()->getMifareCommands()->getSectorFromMAD(
            mLocation->aid, mAiToUse->madKeyA);
    }
    if (mLocation->block == -1)
    {
        mLocation->block = 0;
    }

    size_t totaldatalen = length + (mLocation->block * 16) + mLocation->byte_;
    int nbSectors       = 0;
    size_t totalbuflen  = 0;
    while (totalbuflen < totaldatalen)
    {
        totalbuflen += getMifareChip()->getMifareCommands()->getNbBlocks(
                           mLocation->sector + nbSectors) *
                       16;
        nbSectors++;
    }

    if (nbSectors >= 1)
    {
        ByteVector dataSectors;

        if (behaviorFlags & CB_AUTOSWITCHAREA)
        {
            dataSectors = getMifareChip()->getMifareCommands()->readSectors(
                mLocation->sector, mLocation->sector + nbSectors - 1, mLocation->block,
                mAiToUse->keyA, mAiToUse->keyB, mAiToUse->sab);
        }
        else
        {
            dataSectors = getMifareChip()->getMifareCommands()->readSector(
                mLocation->sector, mLocation->block, mAiToUse->keyA, mAiToUse->keyB,
                mAiToUse->sab);
        }

        if (length <= (dataSectors.size() - mLocation->byte_))
        {
            ret.insert(ret.end(), dataSectors.begin() + mLocation->byte_,
                       dataSectors.begin() + mLocation->byte_ + length);
        }
    }
    return ret;
}

ByteVector MifareStorageCardService::readDataHeader(std::shared_ptr<Location> location,
                                                    std::shared_ptr<AccessInfo> aiToUse)
{
    std::cout << "MifareStorageCardService::readDataHeader" << std::endl;
    TRACE(location, aiToUse);

    EXCEPTION_ASSERT_WITH_LOG(location, std::invalid_argument,
                              "location cannot be null.");

    std::shared_ptr<MifareLocation> mLocation =
        std::dynamic_pointer_cast<MifareLocation>(location);
    EXCEPTION_ASSERT_WITH_LOG(mLocation, std::invalid_argument,
                              "location must be a MifareLocation.");

    MifareKeyType keyType = KT_KEY_A;
    std::shared_ptr<MifareKey> key;

    if (mLocation->block == -1)
    {
        mLocation->block = 0;
    }

    if (aiToUse)
    {
        std::shared_ptr<MifareAccessInfo> mAiToUse =
            std::dynamic_pointer_cast<MifareAccessInfo>(aiToUse);
        EXCEPTION_ASSERT_WITH_LOG(mAiToUse, std::invalid_argument,
                                  "aiToUse must be a MifareAccessInfo");

        keyType = getMifareChip()->getMifareCommands()->getKeyType(
            mAiToUse->sab, mLocation->sector, mLocation->block, false);
        key = keyType == KT_KEY_A ? mAiToUse->keyA : mAiToUse->keyB;
    }

    getMifareChip()->getMifareCommands()->authenticate(
        keyType, key, mLocation->sector,
        getMifareChip()->getMifareCommands()->getNbBlocks(mLocation->sector), false);
    ByteVector vdata = getMifareChip()->getMifareCommands()->readBinary(
        static_cast<unsigned char>(
            getMifareChip()->getMifareCommands()->getSectorStartBlock(mLocation->sector) +
            getMifareChip()->getMifareCommands()->getNbBlocks(mLocation->sector)),
        16);
    return vdata;
}
}