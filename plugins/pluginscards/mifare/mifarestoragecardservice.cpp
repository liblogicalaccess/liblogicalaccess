/**
 * \file mifarestoragecardservice.cpp
 * \author Julien K. <julien-dev@islog.com>, Maxime C. <maxime-dev@islog.com>
 * \brief Mifare storage card service.
 */

#include "mifarestoragecardservice.hpp"
#include "mifarechip.hpp"

namespace logicalaccess
{
    MifareStorageCardService::MifareStorageCardService(std::shared_ptr<Chip> chip)
        : StorageCardService(chip)
    {
    }

    MifareStorageCardService::~MifareStorageCardService()
    {
    }

    void MifareStorageCardService::erase(std::shared_ptr<Location> location, std::shared_ptr<AccessInfo> aiToUse)
    {
        std::shared_ptr<MifareLocation> mLocation = std::dynamic_pointer_cast<MifareLocation>(location);
        if (!mLocation)
        {
            return;
        }

        unsigned int zeroblock_size = getMifareChip()->getMifareCommands()->getNbBlocks(mLocation->sector) * 16;
        unsigned char *zeroblock = new unsigned char[zeroblock_size];
        memset(zeroblock, 0x00, zeroblock_size);

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
            madLocation->block = 1;

            std::shared_ptr<MifareAccessInfo> madAi(new MifareAccessInfo());
            if (aiToUse)
            {
                std::shared_ptr<MifareAccessInfo> mAiToUse = std::dynamic_pointer_cast<MifareAccessInfo>(aiToUse);
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

            writeData(madLocation, madAi, _aiToWrite, zeroblock, 32, CB_DEFAULT);
        }

        bool tmpuseMAD = mLocation->useMAD;
        mLocation->useMAD = false;
        writeData(location, aiToUse, _aiToWrite, zeroblock, zeroblock_size - (mLocation->block * 16), CB_DEFAULT);
        mLocation->useMAD = tmpuseMAD;
        delete[] zeroblock;
    }

    void MifareStorageCardService::writeData(std::shared_ptr<Location> location, std::shared_ptr<AccessInfo> aiToUse, std::shared_ptr<AccessInfo> aiToWrite, const void* data, size_t dataLength, CardBehavior behaviorFlags)
    {
        EXCEPTION_ASSERT_WITH_LOG(location, std::invalid_argument, "location cannot be null.");
        EXCEPTION_ASSERT_WITH_LOG(data, std::invalid_argument, "data cannot be null.");

        std::shared_ptr<MifareLocation> mLocation = std::dynamic_pointer_cast<MifareLocation>(location);
        std::shared_ptr<MifareAccessInfo> mAiToUse = std::dynamic_pointer_cast<MifareAccessInfo>(aiToUse);

        EXCEPTION_ASSERT_WITH_LOG(mLocation, std::invalid_argument, "location must be a MifareLocation.");

        if (aiToUse)
        {
            EXCEPTION_ASSERT_WITH_LOG(mAiToUse, std::invalid_argument, "aiToUse must be a MifareAccessInfo.");
        }
        else
        {
            mAiToUse = std::dynamic_pointer_cast<MifareAccessInfo>(getChip()->getProfile()->createAccessInfo());
        }

        bool writeAidToMad = false;
        getChip()->getProfile()->clearKeys();

        if (mLocation->useMAD)
        {
            if (mLocation->sector == 0)
            {
                mLocation->sector = getMifareChip()->getMifareCommands()->getSectorFromMAD(mLocation->aid, mAiToUse->madKeyA);
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
            totalbuflen += getMifareChip()->getMifareCommands()->getNbBlocks(mLocation->sector + nbSectors) * 16;
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
                    getMifareChip()->getMifareCommands()->setSectorToMAD(mLocation->aid, i, mAiToUse->madKeyA, mAiToUse->madKeyB);
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

            std::shared_ptr<MifareAccessInfo> mAiToWrite;
            // Write access informations too
            if (aiToWrite)
            {
                // Sector keys will be changed after writing
                mAiToWrite = std::dynamic_pointer_cast<MifareAccessInfo>(aiToWrite);

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

                    getMifareChip()->getMifareCommands()->changeKey(mAiToWrite->madKeyA, mAiToWrite->madKeyB, madsector, sab, &newsab, mAiToWrite->madGPB);
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

            if (behaviorFlags & CB_AUTOSWITCHAREA)
            {
                getMifareChip()->getMifareCommands()->writeSectors(mLocation->sector,
                    mLocation->sector + nbSectors - 1,
                    mLocation->block,
                    &dataSectors[0],
                    dataSectors.size(),
                    mAiToUse->sab,
                    mAiToUse->gpb,
                    (mAiToWrite) ? &(mAiToWrite->sab) : NULL);
            }
            else
            {
                getMifareChip()->getMifareCommands()->writeSector(mLocation->sector,
                    mLocation->block,
                    &dataSectors[0],
                    dataSectors.size(),
                    mAiToUse->sab,
                    mAiToUse->gpb,
                    (mAiToWrite) ? &(mAiToWrite->sab) : NULL);
            }

            getMifareChip()->getMifareProfile()->setKeyUsage(getMifareChip()->getNbSectors(), KT_KEY_A, false);
            getMifareChip()->getMifareProfile()->setKeyUsage(getMifareChip()->getNbSectors(), KT_KEY_B, false);
        }
    }

    void MifareStorageCardService::readData(std::shared_ptr<Location> location, std::shared_ptr<AccessInfo> aiToUse, void* data, size_t dataLength, CardBehavior behaviorFlags)
    {
        EXCEPTION_ASSERT_WITH_LOG(location, std::invalid_argument, "location cannot be null.");
        EXCEPTION_ASSERT_WITH_LOG(data, std::invalid_argument, "data cannot be null.");

        std::shared_ptr<MifareLocation> mLocation = std::dynamic_pointer_cast<MifareLocation>(location);
        std::shared_ptr<MifareAccessInfo> mAiToUse = std::dynamic_pointer_cast<MifareAccessInfo>(aiToUse);

        EXCEPTION_ASSERT_WITH_LOG(mLocation, std::invalid_argument, "location must be a MifareLocation.");
        if (aiToUse)
        {
            EXCEPTION_ASSERT_WITH_LOG(mAiToUse, std::invalid_argument, "aiToUse must be a MifareAccessInfo.");
        }
        else
        {
            mAiToUse = std::dynamic_pointer_cast<MifareAccessInfo>(getChip()->getProfile()->createAccessInfo());
        }

        getChip()->getProfile()->clearKeys();

        if (mLocation->useMAD)
        {
            mLocation->sector = getMifareChip()->getMifareCommands()->getSectorFromMAD(mLocation->aid, mAiToUse->madKeyA);
        }

        size_t totaldatalen = dataLength + (mLocation->block * 16) + mLocation->byte;
        int nbSectors = 0;
        size_t totalbuflen = 0;
        while (totalbuflen < totaldatalen)
        {
            totalbuflen += getMifareChip()->getMifareCommands()->getNbBlocks(mLocation->sector + nbSectors) * 16;
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
                reallen = getMifareChip()->getMifareCommands()->readSectors(mLocation->sector, mLocation->sector + nbSectors - 1, mLocation->block, dataSectors.data(), dataSectors.size(), mAiToUse->sab);
            }
            else
            {
                reallen = getMifareChip()->getMifareCommands()->readSector(mLocation->sector, mLocation->block, dataSectors.data(), dataSectors.size(), mAiToUse->sab);
            }

            if (dataLength <= (reallen - mLocation->byte))
            {
                memcpy(static_cast<char*>(data), &dataSectors[0] + mLocation->byte, dataLength);
            }
        }
    }

    unsigned int MifareStorageCardService::readDataHeader(std::shared_ptr<Location> location, std::shared_ptr<AccessInfo> aiToUse, void* data, size_t dataLength)
    {
        if (data == NULL || dataLength == 0)
        {
            return 16;
        }

        EXCEPTION_ASSERT_WITH_LOG(location, std::invalid_argument, "location cannot be null.");

        std::shared_ptr<MifareLocation> mLocation = std::dynamic_pointer_cast<MifareLocation>(location);
        EXCEPTION_ASSERT_WITH_LOG(mLocation, std::invalid_argument, "location must be a MifareLocation.");

        MifareAccessInfo::SectorAccessBits sab;

        if (aiToUse)
        {
            std::shared_ptr<MifareAccessInfo> mAiToUse = std::dynamic_pointer_cast<MifareAccessInfo>(aiToUse);

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
            getMifareChip()->getMifareCommands()->changeBlock(sab, mLocation->sector, getMifareChip()->getMifareCommands()->getNbBlocks(mLocation->sector), false);
            return static_cast<unsigned int>(getMifareChip()->getMifareCommands()->readBinary(static_cast<unsigned char>(getMifareChip()->getMifareCommands()->getSectorStartBlock(mLocation->sector) + getMifareChip()->getMifareCommands()->getNbBlocks(mLocation->sector)), 16, data, 16));
        }

        return 0;
    }

    void MifareStorageCardService::erase()
    {
        unsigned char zeroblock[16];
        unsigned char trailerblock[16];

        memset(zeroblock, 0x00, 16);
        memset(trailerblock, 0xFF, 16);

        MifareAccessInfo::SectorAccessBits sab;
        sab.setTransportConfiguration();
        trailerblock[9] = 0x00;

        if (!sab.toArray(&trailerblock[6], 3))
        {
            THROW_EXCEPTION_WITH_LOG(std::invalid_argument, "Bad sector access bits configuration.");
        }

        for (unsigned int i = 0; i < getMifareChip()->getNbSectors(); ++i)
        {
            bool erased = true;
            bool used = false;
            unsigned int firstBlock = (i == 0) ? 1 : 0; // Don't write the first block in sector 0

            std::shared_ptr<MifareLocation> location(new MifareLocation());
            location->sector = i;
            if (getMifareChip()->getMifareProfile()->getKeyUsage(i, KT_KEY_B))
            {
                used = true;

                std::shared_ptr<MifareKey> key = getMifareChip()->getMifareProfile()->getKey(i, KT_KEY_B);

                getMifareChip()->getMifareCommands()->loadKey(location, key, KT_KEY_B);
                getMifareChip()->getMifareCommands()->authenticate(static_cast<unsigned char>(getMifareChip()->getMifareCommands()->getSectorStartBlock(i)), key->getKeyStorage(), KT_KEY_B);

                for (unsigned int j = firstBlock; j < getMifareChip()->getMifareCommands()->getNbBlocks(i); ++j)
                {
                    if (!getMifareChip()->getMifareCommands()->updateBinary(static_cast<unsigned char>(getMifareChip()->getMifareCommands()->getSectorStartBlock(i) + j), zeroblock, 16))
                    {
                        erased = false;
                    }
                }

                if (!getMifareChip()->getMifareCommands()->updateBinary(static_cast<unsigned char>(getMifareChip()->getMifareCommands()->getSectorStartBlock(i) + getMifareChip()->getMifareCommands()->getNbBlocks(i)), trailerblock, 16))
                {
                    erased = false;
                }
            }

            if ((!erased || !used) && getMifareChip()->getMifareProfile()->getKeyUsage(i, KT_KEY_A))
            {
                used = true;

                std::shared_ptr<MifareKey> key = getMifareChip()->getMifareProfile()->getKey(i, KT_KEY_A);

                getMifareChip()->getMifareCommands()->loadKey(location, key, KT_KEY_A);
                getMifareChip()->getMifareCommands()->authenticate(static_cast<unsigned char>(getMifareChip()->getMifareCommands()->getSectorStartBlock(i)), key->getKeyStorage(), KT_KEY_A);

                for (unsigned int j = firstBlock; j < getMifareChip()->getMifareCommands()->getNbBlocks(i); ++j)
                {
                    if (!getMifareChip()->getMifareCommands()->updateBinary(static_cast<unsigned char>(getMifareChip()->getMifareCommands()->getSectorStartBlock(i) + j), zeroblock, 16))
                    {
                        erased = false;
                    }
                }

                if (!getMifareChip()->getMifareCommands()->updateBinary(static_cast<unsigned char>(getMifareChip()->getMifareCommands()->getSectorStartBlock(i) + getMifareChip()->getMifareCommands()->getNbBlocks(i)), trailerblock, 16))
                {
                    erased = false;
                }
            }
        }
    }
}