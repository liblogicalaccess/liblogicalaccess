/**
 * \file mifareplussl3commands.cpp
 * \author Xavier SCHMERBER <xavier.schmerber@gmail.com>
 * \brief MifarePlus SL3 commands.
 */

#include "mifareplussl3commands.hpp"
#include "mifarepluschip.hpp"

namespace logicalaccess
{
    void MifarePlusSL3Commands::writeSector(int sector, int start_block, const std::vector<unsigned char>& buf, std::shared_ptr<MifarePlusKey> key, MifarePlusKeyType keytype)
    {
        if (buf.size() < MIFARE_PLUS_BLOCK_SIZE || buf.size() % MIFARE_PLUS_BLOCK_SIZE != 0 || buf.size() / MIFARE_PLUS_BLOCK_SIZE + start_block > getNbBlocks(sector))
        {
            THROW_EXCEPTION_WITH_LOG(std::invalid_argument, "Bad buffer parameter. The minimum buffer's length is 16 bytes.");
        }

        if (authenticate(sector, key, keytype))
        {
            updateBinary(getBlockNo(sector, start_block), false, true, buf);
        }
    }

    void MifarePlusSL3Commands::writeSectors(int start_sector, int stop_sector, const std::vector<unsigned char>& buf, std::shared_ptr<AccessInfo> aiToUse)
    {
        if (aiToUse)
        {
            int i;
            size_t test_buflen = 0;
            std::shared_ptr<MifarePlusAccessInfo> mAiToUse = std::dynamic_pointer_cast<MifarePlusAccessInfo>(aiToUse);
            EXCEPTION_ASSERT(mAiToUse, std::invalid_argument, "aiToUse must be a MifarePlusAccessInfo.");

            for (i = start_sector; i < stop_sector; ++i)
            {
                test_buflen += getNbBlocks(i) * MIFARE_PLUS_BLOCK_SIZE;
            }
            if (test_buflen + MIFARE_PLUS_BLOCK_SIZE != buf.size())
                THROW_EXCEPTION_WITH_LOG(std::invalid_argument, "Bad buffer parameter. The buffer must fit the sectors size.");

            test_buflen = 0;
            for (i = start_sector; i <= stop_sector && test_buflen < buf.size(); ++i)
            {
                size_t toWriteLen = getNbBlocks(i) * MIFARE_PLUS_BLOCK_SIZE;
                if (toWriteLen + test_buflen > buf.size())
                    toWriteLen = buf.size() - test_buflen;
				std::vector<unsigned char> tmp(buf.begin() + test_buflen, buf.begin() + test_buflen + toWriteLen);
                if (mAiToUse->keyB && !mAiToUse->keyB->isEmpty())
                    writeSector(i, 0, tmp, mAiToUse->keyB, KT_KEY_AES_B);
                else if (mAiToUse->keyA && !mAiToUse->keyA->isEmpty())
                    writeSector(i, 0, tmp, mAiToUse->keyA, KT_KEY_AES_A);
                else
                    THROW_EXCEPTION_WITH_LOG(std::invalid_argument, "You must set the writing key using the MifarePlusAccessInfo ");
                test_buflen += getNbBlocks(i) * MIFARE_PLUS_BLOCK_SIZE;
            }
        }
    }

    std::vector<unsigned char> MifarePlusSL3Commands::readSector(int sector, int start_block, std::shared_ptr<MifarePlusKey> key, MifarePlusKeyType keytype)
    {
        if (authenticate(sector, key, keytype))
        {
            return readBinary(getBlockNo(sector, start_block), MIFARE_PLUS_BLOCK_SIZE, false, true, true);
        }
		return std::vector<unsigned char>();
    }

    std::vector<unsigned char> MifarePlusSL3Commands::readSectors(int start_sector, int stop_sector, std::shared_ptr<AccessInfo> aiToUse)
    {
		std::vector<unsigned char> ret, data;

        if (aiToUse)
        {
            std::shared_ptr<MifarePlusAccessInfo> mAiToUse = std::dynamic_pointer_cast<MifarePlusAccessInfo>(aiToUse);
            EXCEPTION_ASSERT(mAiToUse, std::invalid_argument, "aiToUse must be a MifarePlusAccessInfo.");

            for (int i = start_sector; i <= stop_sector; ++i)
            {
                if (mAiToUse->keyA && !mAiToUse->keyA->isEmpty())
					data = readSector(i, 0, mAiToUse->keyA, KT_KEY_AES_A);
                else if (mAiToUse->keyB && !mAiToUse->keyB->isEmpty())
                    data = readSector(i, 0, mAiToUse->keyB, KT_KEY_AES_B);
                else
                    THROW_EXCEPTION_WITH_LOG(std::invalid_argument, "You must set the writing key using the MifarePlusAccessInfo ");

				ret.insert(ret.end(), data.begin(), data.end());
            }
        }
		return ret;
    }

    bool MifarePlusSL3Commands::authenticate(std::shared_ptr<Location> location, std::shared_ptr<AccessInfo> ai)
    {
        std::shared_ptr<MifarePlusAccessInfo> mAiToUse = std::dynamic_pointer_cast<MifarePlusAccessInfo>(ai);

        if (ai)
        {
            EXCEPTION_ASSERT(mAiToUse, std::invalid_argument, "ai must be a MifarePlusAccessInfo.");
        }
        else
        {
            mAiToUse = std::dynamic_pointer_cast<MifarePlusAccessInfo>(getChip()->getProfile()->createAccessInfo());
        }

        if (mAiToUse->keyConfiguration && !mAiToUse->keyConfiguration->isEmpty())
            authenticate(0, mAiToUse->keyConfiguration, KT_KEY_CONFIGURATION);
        else if (mAiToUse->keyMastercard && !mAiToUse->keyMastercard->isEmpty())
            authenticate(0, mAiToUse->keyMastercard, KT_KEY_MASTERCARD);
        if (mAiToUse->keyOriginality && !mAiToUse->keyOriginality->isEmpty())
            authenticate(0, mAiToUse->keyOriginality, KT_KEY_ORIGINALITY);
        if (mAiToUse->keyA && !mAiToUse->keyA->isEmpty())
        {
            EXCEPTION_ASSERT(location, std::invalid_argument, "location cannot be null.");
            std::shared_ptr<MifarePlusLocation> mLocation = std::dynamic_pointer_cast<MifarePlusLocation>(location);
            EXCEPTION_ASSERT(mLocation, std::invalid_argument, "location must be a MifarePlusLocation.");

            authenticate(mLocation->sector, mAiToUse->keyA, KT_KEY_AES_A);
        }
        if (mAiToUse->keyB && !mAiToUse->keyB->isEmpty())
        {
            EXCEPTION_ASSERT(location, std::invalid_argument, "location cannot be null.");
            std::shared_ptr<MifarePlusLocation> mLocation = std::dynamic_pointer_cast<MifarePlusLocation>(location);
            EXCEPTION_ASSERT(mLocation, std::invalid_argument, "location must be a MifarePlusLocation.");

            authenticate(mLocation->sector, mAiToUse->keyB, KT_KEY_AES_B);
        }

        return true;
    }

    unsigned char MifarePlusSL3Commands::getNbBlocks(int sector)
    {
        return ((sector >= 32) ? 15 : 3);
    }

    unsigned short MifarePlusSL3Commands::getBlockNo(int sector, int block)
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