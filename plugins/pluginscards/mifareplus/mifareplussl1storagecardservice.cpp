/**
 * \file mifareplussl1storagecardservice.cpp
 * \author Xavier SCHMERBER <xavier.schmerber@gmail.com>
 * \brief MifarePlus SL1 storage card service.
 */

#include "mifareplussl1storagecardservice.hpp"
#include "mifarepluschip.hpp"
#include "logicalaccess/services/accesscontrol/accesscontrolcardservice.hpp"
#include "mifarepluskey.hpp"

namespace logicalaccess
{
    MifarePlusSL1StorageCardService::MifarePlusSL1StorageCardService(std::shared_ptr<Chip> chip)
        : StorageCardService(chip)
    {
    }

    MifarePlusSL1StorageCardService::~MifarePlusSL1StorageCardService()
    {
    }

    void MifarePlusSL1StorageCardService::erase(std::shared_ptr<Location> location, std::shared_ptr<AccessInfo> aiToUse)
    {
        std::shared_ptr<MifarePlusLocation> mLocation = std::dynamic_pointer_cast<MifarePlusLocation>(location);
        if (!mLocation)
        {
            return;
        }

        unsigned int zeroblock_size = getMifarePlusChip()->getMifarePlusSL1Commands()->getNbBlocks(mLocation->sector) * 16;

        std::shared_ptr<MifarePlusAccessInfo> _aiToWrite;
        _aiToWrite.reset(new MifarePlusAccessInfo(MIFARE_PLUS_CRYPTO1_KEY_SIZE));

        if (mLocation->sector == 0)
        {
            if (mLocation->block == 0)
            {
                mLocation->block = 1;
            }
        }
        else if (mLocation->useMAD)
        {
            std::shared_ptr<MifarePlusLocation> madLocation(new MifarePlusLocation());
            madLocation->sector = 0;
            madLocation->block = 1;

            std::shared_ptr<MifarePlusAccessInfo> madAi(new MifarePlusAccessInfo(MIFARE_PLUS_CRYPTO1_KEY_SIZE));
            if (aiToUse)
            {
                std::shared_ptr<MifarePlusAccessInfo> mAiToUse = std::dynamic_pointer_cast<MifarePlusAccessInfo>(aiToUse);
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

			std::vector<unsigned char> zeroblock(zeroblock_size, 0x00);
            writeData(madLocation, madAi, _aiToWrite, zeroblock, CB_DEFAULT);
        }

        bool tmpuseMAD = mLocation->useMAD;
        mLocation->useMAD = false;
		std::vector<unsigned char> zeroblock(32 - (mLocation->block * 16), 0x00);
        writeData(location, aiToUse, _aiToWrite, zeroblock, CB_DEFAULT);
        mLocation->useMAD = tmpuseMAD;
    }

    void MifarePlusSL1StorageCardService::writeData(std::shared_ptr<Location> location, std::shared_ptr<AccessInfo> aiToUse, std::shared_ptr<AccessInfo> aiToWrite, const std::vector<unsigned char>& data, CardBehavior behaviorFlags)
    {
        EXCEPTION_ASSERT_WITH_LOG(location, std::invalid_argument, "location cannot be null.");

        std::shared_ptr<MifarePlusLocation> mLocation = std::dynamic_pointer_cast<MifarePlusLocation>(location);
        std::shared_ptr<MifarePlusAccessInfo> mAiToUse = std::dynamic_pointer_cast<MifarePlusAccessInfo>(aiToUse);

        EXCEPTION_ASSERT_WITH_LOG(mLocation, std::invalid_argument, "location must be a MifareLocation.");

        if (aiToUse)
        {
            EXCEPTION_ASSERT_WITH_LOG(mAiToUse, std::invalid_argument, "aiToUse must be a MifareAccessInfo.");
        }
        else
        {
            mAiToUse = std::dynamic_pointer_cast<MifarePlusAccessInfo>(getChip()->getProfile()->createAccessInfo());
        }

        bool writeAidToMad = false;
        getChip()->getProfile()->clearKeys();

        if (mLocation->useMAD)
        {
            if (mLocation->sector == 0)
            {
                mLocation->sector = getMifarePlusChip()->getMifarePlusSL1Commands()->getSectorFromMAD(mLocation->aid, mAiToUse->madKeyA);
            }
            else
            {
                writeAidToMad = true;
            }
        }

        size_t totaldatalen = data.size() + (mLocation->block * 16) + mLocation->byte;
        int nbSectors = 0;
        size_t totalbuflen = 0;
        while (totalbuflen < totaldatalen)
        {
            totalbuflen += getMifarePlusChip()->getMifarePlusSL1Commands()->getNbBlocks(mLocation->sector + nbSectors) * 16;
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
            std::vector<unsigned char> dataSectors(buflen, 0x00);
			std::copy(data.begin(), data.end(), dataSectors.begin() + mLocation->byte);

            if (writeAidToMad)
            {
                for (int i = mLocation->sector; i < (mLocation->sector + nbSectors); ++i)
                {
                    getMifarePlusChip()->getMifarePlusSL1Commands()->setSectorToMAD(mLocation->aid, i, mAiToUse->madKeyA, mAiToUse->madKeyB);
                }
            }

            for (int i = 0; i < nbSectors; ++i)
            {
                if (!mAiToUse->keyA->isEmpty())
                {
                    std::dynamic_pointer_cast<MifarePlusSL1Profile>(getChip()->getProfile())->setKey(mLocation->sector + i, KT_KEY_CRYPTO1_A, mAiToUse->keyA);
                }
                if (!mAiToUse->keyB->isEmpty())
                {
                    std::dynamic_pointer_cast<MifarePlusSL1Profile>(getChip()->getProfile())->setKey(mLocation->sector + i, KT_KEY_CRYPTO1_B, mAiToUse->keyB);
                }
            }

            std::shared_ptr<MifarePlusAccessInfo> mAiToWrite;
            // Write access informations too
            if (aiToWrite)
            {
                // Sector keys will be changed after writing
                mAiToWrite = std::dynamic_pointer_cast<MifarePlusAccessInfo>(aiToWrite);

                EXCEPTION_ASSERT_WITH_LOG(mAiToWrite, std::invalid_argument, "mAiToWrite must be a MifarePlusAccessInfo.");

                // MAD keys are changed now
                if (writeAidToMad && mAiToWrite->useMAD)
                {
                    unsigned int madsector = (mLocation->sector <= 16) ? 0 : 16;
                    MifarePlusAccessInfo::SectorAccessBits sab;
                    std::dynamic_pointer_cast<MifarePlusSL1Profile>(getChip()->getProfile())->setKey(madsector, KT_KEY_CRYPTO1_A, mAiToUse->madKeyA);
                    if (mAiToUse->madKeyB->isEmpty())
                    {
                        sab.setTransportConfiguration();
                    }
                    else
                    {
                        sab.setAReadBWriteConfiguration();
                        std::dynamic_pointer_cast<MifarePlusSL1Profile>(getChip()->getProfile())->setKey(madsector, KT_KEY_CRYPTO1_B, mAiToUse->madKeyB);
                    }

                    MifarePlusAccessInfo::SectorAccessBits newsab;
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
                        std::dynamic_pointer_cast<MifarePlusSL1Profile>(getChip()->getProfile())->setKey(getMifarePlusChip()->getNbSectors(), KT_KEY_CRYPTO1_A, mAiToWrite->madKeyA);
                    }

                    if (!mAiToWrite->madKeyB->isEmpty())
                    {
                        std::dynamic_pointer_cast<MifarePlusSL1Profile>(getChip()->getProfile())->setKey(getMifarePlusChip()->getNbSectors(), KT_KEY_CRYPTO1_B, mAiToWrite->madKeyB);
                    }

                    unsigned char userbyte = 0xC3; // DA: 1, MA: 1, RFU, ADV: 01
                    getMifarePlusChip()->getMifarePlusSL1Commands()->changeKey(mAiToWrite->madKeyA, mAiToWrite->madKeyB, madsector, sab, &newsab, userbyte);
                }

                if (!mAiToWrite->keyA->isEmpty())
                {
                    std::dynamic_pointer_cast<MifarePlusSL1Profile>(getChip()->getProfile())->setKey(getMifarePlusChip()->getNbSectors(), KT_KEY_CRYPTO1_A, mAiToWrite->keyA);
                }

                if (!mAiToWrite->keyB->isEmpty())
                {
                    std::dynamic_pointer_cast<MifarePlusSL1Profile>(getChip()->getProfile())->setKey(getMifarePlusChip()->getNbSectors(), KT_KEY_CRYPTO1_B, mAiToWrite->keyB);
                }
            }

            if (behaviorFlags & CB_AUTOSWITCHAREA)
            {
                getMifarePlusChip()->getMifarePlusSL1Commands()->writeSectors(mLocation->sector,
                    mLocation->sector + nbSectors - 1,
                    mLocation->block,
                    dataSectors,
                    mAiToUse->sab,
                    mAiToUse->gpb,
                    (mAiToWrite) ? &(mAiToWrite->sab) : NULL);
            }
            else
            {
                getMifarePlusChip()->getMifarePlusSL1Commands()->writeSector(mLocation->sector,
                    mLocation->block,
                    dataSectors,
                    mAiToUse->sab,
                    mAiToUse->gpb,
                    (mAiToWrite) ? &(mAiToWrite->sab) : NULL);
            }

            std::dynamic_pointer_cast<MifarePlusSL1Profile>(getChip()->getProfile())->setKeyUsage(getMifarePlusChip()->getNbSectors(), KT_KEY_CRYPTO1_A, false);
            std::dynamic_pointer_cast<MifarePlusSL1Profile>(getChip()->getProfile())->setKeyUsage(getMifarePlusChip()->getNbSectors(), KT_KEY_CRYPTO1_B, false);
        }
    }

    std::vector<unsigned char> MifarePlusSL1StorageCardService::readData(std::shared_ptr<Location> location, std::shared_ptr<AccessInfo> aiToUse, size_t length, CardBehavior behaviorFlags)
    {
        EXCEPTION_ASSERT_WITH_LOG(location, std::invalid_argument, "location cannot be null.");
		std::vector<unsigned char> ret;
        std::shared_ptr<MifarePlusLocation> mLocation = std::dynamic_pointer_cast<MifarePlusLocation>(location);
        std::shared_ptr<MifarePlusAccessInfo> mAiToUse = std::dynamic_pointer_cast<MifarePlusAccessInfo>(aiToUse);

        EXCEPTION_ASSERT_WITH_LOG(mLocation, std::invalid_argument, "location must be a MifarePlusLocation.");
        if (aiToUse)
        {
            EXCEPTION_ASSERT_WITH_LOG(mAiToUse, std::invalid_argument, "aiToUse must be a MifarePlusAccessInfo.");
        }
        else
        {
            mAiToUse = std::dynamic_pointer_cast<MifarePlusAccessInfo>(getChip()->getProfile()->createAccessInfo());
        }

        getChip()->getProfile()->clearKeys();

        if (mLocation->useMAD)
        {
            mLocation->sector = getMifarePlusChip()->getMifarePlusSL1Commands()->getSectorFromMAD(mLocation->aid, mAiToUse->madKeyA);
        }

		size_t totaldatalen = length + (mLocation->block * 16) + mLocation->byte;
        int nbSectors = 0;
        size_t totalbuflen = 0;
        while (totalbuflen < totaldatalen)
        {
            totalbuflen += getMifarePlusChip()->getMifarePlusSL1Commands()->getNbBlocks(mLocation->sector + nbSectors) * 16;
            nbSectors++;
        }

        for (int i = 0; i < nbSectors; ++i)
        {
            if (!mAiToUse->keyA->isEmpty())
            {
                std::dynamic_pointer_cast<MifarePlusSL1Profile>(getChip()->getProfile())->setKey(mLocation->sector + i, KT_KEY_CRYPTO1_A, mAiToUse->keyA);
            }
            if (!mAiToUse->keyB->isEmpty())
            {
                std::dynamic_pointer_cast<MifarePlusSL1Profile>(getChip()->getProfile())->setKey(mLocation->sector + i, KT_KEY_CRYPTO1_B, mAiToUse->keyB);
            }
        }

        if (nbSectors >= 1)
        {
            std::vector<unsigned char> dataSectors;

            if (behaviorFlags & CB_AUTOSWITCHAREA)
            {
                dataSectors = getMifarePlusChip()->getMifarePlusSL1Commands()->readSectors(mLocation->sector, mLocation->sector + nbSectors - 1, mLocation->block, mAiToUse->sab);
            }
            else
            {
                dataSectors = getMifarePlusChip()->getMifarePlusSL1Commands()->readSector(mLocation->sector, mLocation->block, mAiToUse->sab);
            }

			if (length <= (dataSectors.size() - mLocation->byte))
            {
				ret.insert(ret.end(), dataSectors.begin() + mLocation->byte, dataSectors.begin() + mLocation->byte + length);
            }
        }
		return ret;
    }

    unsigned int MifarePlusSL1StorageCardService::readDataHeader(std::shared_ptr<Location> location, std::shared_ptr<AccessInfo> aiToUse, void* data, size_t dataLength)
    {
        if (data == NULL || dataLength == 0)
        {
            return 16;
        }

        EXCEPTION_ASSERT_WITH_LOG(location, std::invalid_argument, "location cannot be null.");

        std::shared_ptr<MifarePlusLocation> mLocation = std::dynamic_pointer_cast<MifarePlusLocation>(location);
        EXCEPTION_ASSERT_WITH_LOG(mLocation, std::invalid_argument, "location must be a MifarePlusLocation.");

        MifarePlusAccessInfo::SectorAccessBits sab;

        if (aiToUse)
        {
            std::shared_ptr<MifarePlusAccessInfo> mAiToUse = std::dynamic_pointer_cast<MifarePlusAccessInfo>(aiToUse);

            EXCEPTION_ASSERT_WITH_LOG(mAiToUse, std::invalid_argument, "aiToUse must be a MifarePlusAccessInfo");

            if (!mAiToUse->keyA->isEmpty())
            {
                std::dynamic_pointer_cast<MifarePlusSL1Profile>(getChip()->getProfile())->setKey(mLocation->sector, KT_KEY_CRYPTO1_A, mAiToUse->keyA);
            }

            if (!mAiToUse->keyB->isEmpty())
            {
                std::dynamic_pointer_cast<MifarePlusSL1Profile>(getChip()->getProfile())->setKey(mLocation->sector, KT_KEY_CRYPTO1_B, mAiToUse->keyB);
            }

            sab = mAiToUse->sab;
        }
        else
        {
            std::dynamic_pointer_cast<MifarePlusProfile>(getChip()->getProfile())->setDefaultKeysAt(mLocation->sector);
            sab.setTransportConfiguration();
        }

        if (dataLength >= 16)
        {
            getMifarePlusChip()->getMifarePlusSL1Commands()->changeBlock(sab, mLocation->sector, getMifarePlusChip()->getMifarePlusSL1Commands()->getNbBlocks(mLocation->sector), false);
            std::vector<unsigned char> tmp = getMifarePlusChip()->getMifarePlusSL1Commands()->readBinary(static_cast<unsigned char>(getMifarePlusChip()->getMifarePlusSL1Commands()->getSectorStartBlock(mLocation->sector) + getMifarePlusChip()->getMifarePlusSL1Commands()->getNbBlocks(mLocation->sector)), 16);
			memcpy(data, &tmp[0], tmp.size() > dataLength ? dataLength : tmp.size());
			return tmp.size();
        }
        return 0;
    }

    void MifarePlusSL1StorageCardService::erase()
    {
        std::vector<unsigned char> zeroblock(16, 0x00);
		std::vector<unsigned char> trailerblock(16, 0xFF);

        MifarePlusAccessInfo::SectorAccessBits sab;
        sab.setTransportConfiguration();
        trailerblock[9] = 0x00;

        if (!sab.toArray(&trailerblock[6], 3))
        {
            THROW_EXCEPTION_WITH_LOG(std::invalid_argument, "Bad sector access bits configuration.");
        }

        for (unsigned int i = 0; i < getMifarePlusChip()->getNbSectors(); ++i)
        {
            bool erased = true;
            bool used = false;
            unsigned int firstBlock = (i == 0) ? 1 : 0; // Don't write the first block in sector 0

            std::shared_ptr<MifarePlusLocation> location(new MifarePlusLocation());
            location->sector = i;
            if (std::dynamic_pointer_cast<MifarePlusSL1Profile>(getChip()->getProfile())->getKeyUsage(i, KT_KEY_CRYPTO1_B))
            {
                used = true;

                std::shared_ptr<MifarePlusKey> key = std::dynamic_pointer_cast<MifarePlusSL1Profile>(getChip()->getProfile())->getKey(i, KT_KEY_CRYPTO1_B);

                getMifarePlusChip()->getMifarePlusSL1Commands()->loadKey(location, key, KT_KEY_CRYPTO1_B);
                getMifarePlusChip()->getMifarePlusSL1Commands()->authenticate(static_cast<unsigned char>(getMifarePlusChip()->getMifarePlusSL1Commands()->getSectorStartBlock(i)), key->getKeyStorage(), KT_KEY_CRYPTO1_B);

                for (unsigned int j = firstBlock; j < getMifarePlusChip()->getMifarePlusSL1Commands()->getNbBlocks(i); ++j)
                {
                    getMifarePlusChip()->getMifarePlusSL1Commands()->updateBinary(static_cast<unsigned char>(getMifarePlusChip()->getMifarePlusSL1Commands()->getSectorStartBlock(i) + j), zeroblock);
                }

                getMifarePlusChip()->getMifarePlusSL1Commands()->updateBinary(static_cast<unsigned char>(getMifarePlusChip()->getMifarePlusSL1Commands()->getSectorStartBlock(i) + getMifarePlusChip()->getMifarePlusSL1Commands()->getNbBlocks(i)), trailerblock);
            }

            if ((!erased || !used) && std::dynamic_pointer_cast<MifarePlusSL1Profile>(getChip()->getProfile())->getKeyUsage(i, KT_KEY_CRYPTO1_A))
            {
                used = true;

                std::shared_ptr<MifarePlusKey> key = std::dynamic_pointer_cast<MifarePlusSL1Profile>(getChip()->getProfile())->getKey(i, KT_KEY_CRYPTO1_A);

                getMifarePlusChip()->getMifarePlusSL1Commands()->loadKey(location, key, KT_KEY_CRYPTO1_A);
                getMifarePlusChip()->getMifarePlusSL1Commands()->authenticate(static_cast<unsigned char>(getMifarePlusChip()->getMifarePlusSL1Commands()->getSectorStartBlock(i)), key->getKeyStorage(), KT_KEY_CRYPTO1_A);

                for (unsigned int j = firstBlock; j < getMifarePlusChip()->getMifarePlusSL1Commands()->getNbBlocks(i); ++j)
                {
                    getMifarePlusChip()->getMifarePlusSL1Commands()->updateBinary(static_cast<unsigned char>(getMifarePlusChip()->getMifarePlusSL1Commands()->getSectorStartBlock(i) + j), zeroblock);
                }

                getMifarePlusChip()->getMifarePlusSL1Commands()->updateBinary(static_cast<unsigned char>(getMifarePlusChip()->getMifarePlusSL1Commands()->getSectorStartBlock(i) + getMifarePlusChip()->getMifarePlusSL1Commands()->getNbBlocks(i)), trailerblock);
            }
        }
    }
}