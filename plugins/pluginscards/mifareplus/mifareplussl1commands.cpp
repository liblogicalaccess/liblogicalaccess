/**
 * \file mifareplussl1commands.cpp
 * \author Xavier SCHMERBER <xavier.schmerber@gmail.com>
 * \brief MifarePlus SL1 commands.
 */

#include <cstring>
#include "mifareplussl3commands.hpp"
#include "mifarepluschip.hpp"
#include "logicalaccess/myexception.hpp"

#define PREFIX_PATTERN 0xE3
#define POLYNOM_PATTERN 0x1D

namespace logicalaccess
{
    bool MifarePlusSL1Commands::authenticate(std::shared_ptr<Location> location, std::shared_ptr<AccessInfo> ai)
    {
        EXCEPTION_ASSERT_WITH_LOG(location, std::invalid_argument, "location cannot be null.");
        EXCEPTION_ASSERT_WITH_LOG(ai, std::invalid_argument, "ai cannot be null.");

        std::shared_ptr<MifarePlusLocation> mLocation = std::dynamic_pointer_cast<MifarePlusLocation>(location);
        std::shared_ptr<MifarePlusAccessInfo> mAi = std::dynamic_pointer_cast<MifarePlusAccessInfo>(ai);

        EXCEPTION_ASSERT_WITH_LOG(mLocation, std::invalid_argument, "location must be a MifarePlusLocation.");
        EXCEPTION_ASSERT_WITH_LOG(mAi, std::invalid_argument, "ai must be a MifarePlusAccessInfo.");

        getChip()->getProfile()->clearKeys();

        EXCEPTION_ASSERT_WITH_LOG((!mAi->keyA->isEmpty()) || (!mAi->keyB->isEmpty()), std::invalid_argument, "ai must contain at least one key");

        MifarePlusKeyType keytype = KT_KEY_CRYPTO1_A;

        if (!mAi->keyA->isEmpty())
        {
            std::dynamic_pointer_cast<MifarePlusSL1Profile>(getChip()->getProfile())->setKey(mLocation->sector, KT_KEY_CRYPTO1_A, mAi->keyA);
        }

        if (!mAi->keyB->isEmpty())
        {
            std::dynamic_pointer_cast<MifarePlusSL1Profile>(getChip()->getProfile())->setKey(mLocation->sector, KT_KEY_CRYPTO1_B, mAi->keyB);
            keytype = KT_KEY_CRYPTO1_B;
        }

        if (!std::dynamic_pointer_cast<MifarePlusSL1Profile>(getChip()->getProfile())->getKeyUsage(mLocation->sector, keytype))
        {
            THROW_EXCEPTION_WITH_LOG(CardException, EXCEPTION_MSG_NOKEY);
        }

        std::shared_ptr<MifarePlusKey> key = std::dynamic_pointer_cast<MifarePlusKey>(std::dynamic_pointer_cast<MifarePlusSL1Profile>(getChip()->getProfile())->getKey(mLocation->sector, keytype));
        loadKey(location, key, keytype);
        getMifarePlusChip()->getMifarePlusSL1Commands()->authenticate(static_cast<unsigned char>(getSectorStartBlock(mLocation->sector)), key->getKeyStorage(), keytype);

        return true;
    }
    unsigned int MifarePlusSL1Commands::getSectorFromMAD(long aid, std::shared_ptr<MifarePlusKey> madKeyA)
    {
        unsigned int sector = static_cast<unsigned int>(-1);
        MifarePlusAccessInfo::SectorAccessBits sab;

        if (!madKeyA->isEmpty())
        {
            getMifarePlusChip()->getMifarePlusSL1Profile()->setKey(0, KT_KEY_CRYPTO1_A, madKeyA);
        }

		std::vector<unsigned char> madbuf = readSector(0, 1, sab);
        if (!madbuf.size())
        {
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Can't read the MAD.");
        }

        unsigned char madcrc = calculateMADCrc(&madbuf[0], madbuf.size());
        if (madcrc != madbuf[0])
        {
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Bad MAD CRC.");
        }

        sector = findReferencedSector(aid, &madbuf[0], madbuf.size());

        if ((sector == static_cast<unsigned int>(-1)) && (getChip()->getCardType() == "MifarePlus4K" || getChip()->getCardType() == "MifarePlus2K"))
        {
            std::vector<unsigned char> madbuf = readSector(16, 0, sab);
			if (!madbuf.size())
            {
                THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Can't read the MAD2.");
            }

            unsigned char mad2crc = calculateMADCrc(&madbuf[0], madbuf.size());
            if (mad2crc != madbuf[0])
            {
                THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Bad MAD2 CRC.");
            }

            sector = findReferencedSector(aid, &madbuf[0], madbuf.size());

            if (sector != static_cast<unsigned int>(-1))
            {
                sector += 16;
            }
        }

        return sector;
    }

    void MifarePlusSL1Commands::setSectorToMAD(long aid, unsigned int sector, std::shared_ptr<MifarePlusKey> madKeyA, std::shared_ptr<MifarePlusKey> madKeyB)
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

            std::vector<unsigned char> madbuf = readSector(0, 1, sab);
			if (!madbuf.size())
            {
                THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Can't read the MAD.");
            }

            madbuf[sector * 2] = (aid & 0xff00) >> 8;
            madbuf[(sector * 2) + 1] = aid & 0xff;
            if (madbuf[1] == 0x00)
            {
                madbuf[1] = static_cast<unsigned char>(0x01 << (sector - 1));
            }
            madbuf[0] = calculateMADCrc(&madbuf[0], madbuf.size());

            writeSector(0, 1, madbuf, sab);
        }
        else
        {
            if (sector == 16)
            {
                THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Can't make reference to the MAD2 itself.");
            }

            std::vector<unsigned char> madbuf = readSector(16, 0, sab);
			if (!madbuf.size())
            {
                THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Can't read the MAD2.");
            }

            sector -= 16;
            madbuf[sector * 2] = (aid & 0xff00) >> 8;
            madbuf[(sector * 2) + 1] = aid & 0xff;
            madbuf[0] = calculateMADCrc(&madbuf[0], madbuf.size());

            writeSector(16, 0, madbuf, sab);
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
            paid = (madbuf[i * 2] << 8) | madbuf[(i * 2) + 1];

            if (paid == aid)
            {
                sector = i;
            }
        }

        return sector;
    }

    std::shared_ptr<MifarePlusChip> MifarePlusSL1Commands::getMifarePlusChip()
    {
        return std::dynamic_pointer_cast<MifarePlusChip>(getChip());
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
            else if (block < 10)
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
        std::shared_ptr<MifarePlusKey> key = std::dynamic_pointer_cast<MifarePlusSL1Profile>(getChip()->getProfile())->getKey(sector, keytype);

        if (!std::dynamic_pointer_cast<MifarePlusSL1Profile>(getChip()->getProfile())->getKeyUsage(sector, keytype))
        {
            THROW_EXCEPTION_WITH_LOG(CardException, EXCEPTION_MSG_NOKEY);
        }

        std::shared_ptr<MifarePlusLocation> location(new MifarePlusLocation());
        location->sector = sector;
        location->block = block;

        loadKey(location, key, keytype);
        authenticate(static_cast<unsigned char>(getSectorStartBlock(sector)), key->getKeyStorage(), keytype);
    }

    std::vector<unsigned char> MifarePlusSL1Commands::readSector(int sector, int start_block, const MifarePlusAccessInfo::SectorAccessBits& sab)
    {
        std::vector<unsigned char> ret;

        for (int i = start_block; i < getNbBlocks(sector); i++)
        {
            changeBlock(sab, sector, i, false);
            std::vector<unsigned char> data = readBinary(static_cast<unsigned char>(getSectorStartBlock(sector) + i), 16);
			ret.insert(ret.end(), data.begin(), data.end());
        }

        return ret;
    }

    void MifarePlusSL1Commands::writeSector(int sector, int start_block, const std::vector<unsigned char>& buf, const MifarePlusAccessInfo::SectorAccessBits& sab, unsigned char userbyte, MifarePlusAccessInfo::SectorAccessBits* newsab)
    {
        size_t retlen = 0;

        if (buf.size() < (getNbBlocks(sector) - static_cast<unsigned int>(start_block)) * 16)
        {
            THROW_EXCEPTION_WITH_LOG(std::invalid_argument, "Bad buffer parameter. The minimum buffer's length is 48 bytes.");
        }

        for (int i = start_block; i < getNbBlocks(sector); i++)
        {
            changeBlock(sab, sector, i, true);
			std::vector<unsigned char> tmp(buf.begin() + retlen, buf.end() + retlen + 16);
            updateBinary(static_cast<unsigned char>(getSectorStartBlock(sector) + i), tmp);
			retlen += 16;
        }

        if (newsab != NULL)
        {
            int writeSector = getMifarePlusChip()->getNbSectors();
            changeKey(std::dynamic_pointer_cast<MifarePlusSL1Profile>(getChip()->getProfile())->getKey(writeSector, KT_KEY_CRYPTO1_A), std::dynamic_pointer_cast<MifarePlusSL1Profile>(getChip()->getProfile())->getKey(writeSector, KT_KEY_CRYPTO1_B), sector, sab, newsab, userbyte);
        }
    }

    void MifarePlusSL1Commands::changeKey(std::shared_ptr<MifarePlusKey> keyA, std::shared_ptr<MifarePlusKey> keyB, unsigned int sector, const MifarePlusAccessInfo::SectorAccessBits& sab, MifarePlusAccessInfo::SectorAccessBits* newsab, unsigned char userbyte)
    {
        std::vector<unsigned char> trailerblock(16, 0x00);

        if (!keyB || keyB->isEmpty())
        {
            memcpy(&trailerblock[0], keyA->getData(), MIFARE_PLUS_CRYPTO1_KEY_SIZE);
        }
        else
        {
            memcpy(&trailerblock[0], keyA->getData(), MIFARE_PLUS_CRYPTO1_KEY_SIZE);
            memcpy(&trailerblock[10], keyB->getData(), MIFARE_PLUS_CRYPTO1_KEY_SIZE);
        }

        if ((*newsab).toArray(&trailerblock[MIFARE_PLUS_CRYPTO1_KEY_SIZE], 3) != 3)
        {
            THROW_EXCEPTION_WITH_LOG(CardException, EXCEPTION_MSG_CHANGEKEY);
        }

        trailerblock[MIFARE_PLUS_CRYPTO1_KEY_SIZE + 3] = userbyte;

        changeBlock(sab, sector, getNbBlocks(sector), true);
        updateBinary(static_cast<unsigned char>(getSectorStartBlock(sector) + getNbBlocks(sector)), trailerblock);
    }

    std::vector<unsigned char> MifarePlusSL1Commands::readSectors(int start_sector, int stop_sector, int start_block, const MifarePlusAccessInfo::SectorAccessBits& sab)
    {
		std::vector<unsigned char> ret;
        if (start_sector > stop_sector)
        {
            THROW_EXCEPTION_WITH_LOG(std::invalid_argument, "Start sector can't be greater than stop sector.");
        }

        for (int i = start_sector; i <= stop_sector; ++i)
        {
            int startBlockSector = (i == start_sector) ? start_block : 0;
            std::vector<unsigned char> tmp = readSector(i, startBlockSector, sab);
			ret.insert(ret.end(), tmp.begin(), tmp.end());
        }

        return ret;
    }

    void MifarePlusSL1Commands::writeSectors(int start_sector, int stop_sector, int start_block, const std::vector<unsigned char>& buf, const MifarePlusAccessInfo::SectorAccessBits& sab, unsigned char userbyte, MifarePlusAccessInfo::SectorAccessBits* newsab)
    {
        if (start_sector > stop_sector)
        {
            THROW_EXCEPTION_WITH_LOG(std::invalid_argument, "Start sector can't be greater than stop sector.");
        }

        size_t offset = 0;

        for (int i = start_sector; i <= stop_sector; ++i)
		{
            int startBlockSector = (i == start_sector) ? start_block : 0;
            std::vector<unsigned char> tmp(buf.begin() + offset, buf.begin() + offset + (getNbBlocks(i) - startBlockSector) * 16);
			writeSector(i, startBlockSector, tmp, sab, userbyte, newsab);

			offset += (getNbBlocks(i) - startBlockSector) * 16;
        }
    }

    unsigned char MifarePlusSL1Commands::getNbBlocks(int sector)
    {
        return ((sector >= 32) ? 15 : 3);
    }

    unsigned char MifarePlusSL1Commands::getSectorStartBlock(int sector)
    {
        unsigned char start_block = 0;
        for (int i = 0; i < sector; ++i)
        {
            start_block += getNbBlocks(i) + 1;
        }

        return start_block;
    }
}