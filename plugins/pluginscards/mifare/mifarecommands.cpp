/**
 * \file mifarecommands.cpp
 * \author Julien K. <julien-dev@islog.com>, Maxime C. <maxime-dev@islog.com>
 * \brief Mifare commands.
 */

#include <cstring>
#include <logicalaccess/logs.hpp>
#include <cassert>
#include "mifarecommands.hpp"
#include "mifarechip.hpp"
#include "logicalaccess/myexception.hpp"

#define PREFIX_PATTERN 0xE3
#define POLYNOM_PATTERN 0x1D

namespace logicalaccess
{
    void MifareCommands::authenticate(std::shared_ptr<Location> location, std::shared_ptr<AccessInfo> ai)
    {
        EXCEPTION_ASSERT_WITH_LOG(location, std::invalid_argument, "location cannot be null.");
        EXCEPTION_ASSERT_WITH_LOG(ai, std::invalid_argument, "ai cannot be null.");

        std::shared_ptr<MifareLocation> mLocation = std::dynamic_pointer_cast<MifareLocation>(location);
        std::shared_ptr<MifareAccessInfo> mAi = std::dynamic_pointer_cast<MifareAccessInfo>(ai);

        EXCEPTION_ASSERT_WITH_LOG(mLocation, std::invalid_argument, "location must be a MifareLocation.");
        EXCEPTION_ASSERT_WITH_LOG(mAi, std::invalid_argument, "ai must be a MifareAccessInfo.");

        getChip()->getProfile()->setDefaultKeysAt(location);

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

        std::shared_ptr<MifareKey> key = getMifareChip()->getMifareProfile()->getKey(mLocation->sector, keytype);
        loadKey(location, key, keytype);
        authenticate(static_cast<unsigned char>(getSectorStartBlock(mLocation->sector)), key->getKeyStorage(), keytype);
    }

    unsigned int MifareCommands::getSectorFromMAD(long aid, std::shared_ptr<MifareKey> madKeyA)
    {
        unsigned int sector = static_cast<unsigned int>(-1);
        MifareAccessInfo::SectorAccessBits sab;

        if (!madKeyA->isEmpty())
        {
            getMifareChip()->getMifareProfile()->setKey(0, KT_KEY_A, madKeyA);
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

        if ((sector == static_cast<unsigned int>(-1)) && getChip()->getCardType() == "Mifare4K")
        {
            madbuf = readSector(16, 0, sab);
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

    void MifareCommands::setSectorToMAD(long aid, unsigned int sector, std::shared_ptr<MifareKey> madKeyA, std::shared_ptr<MifareKey> madKeyB)
    {
        MifareAccessInfo::SectorAccessBits sab;

        if (!madKeyA->isEmpty())
        {
            getMifareChip()->getMifareProfile()->setKey(0, KT_KEY_A, madKeyA);
            getMifareChip()->getMifareProfile()->setKey(16, KT_KEY_A, madKeyA);
        }

        if (madKeyB->isEmpty())
        {
            sab.setTransportConfiguration();
        }
        else
        {
            getMifareChip()->getMifareProfile()->setKey(0, KT_KEY_B, madKeyB);
            getMifareChip()->getMifareProfile()->setKey(16, KT_KEY_B, madKeyB);
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

            madbuf[(sector * 2)] = aid & 0xff;
            madbuf[sector * 2 + 1] = (aid & 0xff00) >> 8;
            if (madbuf[1] == 0x00)
            {
                // BCD Nibble representation
                madbuf[1] = static_cast<unsigned char>(sector % 10);
                if (sector >= 10)
                {
                    madbuf[1] |= 0x10;
                }
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

    unsigned char MifareCommands::calculateMADCrc(const unsigned char* madbuf, size_t madbuflen)
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

    unsigned int MifareCommands::findReferencedSector(long aid, unsigned char* madbuf, size_t madbuflen)
    {
        unsigned int sector = static_cast<unsigned int>(-1);

        for (unsigned int i = 1; i < madbuflen && (sector == static_cast<unsigned int>(-1)); ++i)
        {
            long paid = 0;
            paid = madbuf[(i * 2)] | (madbuf[i * 2 + 1] << 8);

            if (paid == aid)
            {
                sector = i;
            }
        }

        return sector;
    }

    void MifareCommands::changeBlock(const MifareAccessInfo::SectorAccessBits& sab, int sector, int block, bool write)
    {
		TRACE(sector, block, write);
        MifareKeyType wkt = KT_KEY_A;
        MifareKeyType rkt = KT_KEY_A;

        LOG(LogLevel::INFOS) << "mcba1";

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
        std::shared_ptr<MifareKey> key = getMifareChip()->getMifareProfile()->getKey(sector, keytype);

        if (!getMifareChip()->getMifareProfile()->getKeyUsage(sector, keytype))
        {
            THROW_EXCEPTION_WITH_LOG(CardException, EXCEPTION_MSG_NOKEY);
        }

        std::shared_ptr<MifareLocation> location(new MifareLocation());
        location->sector = sector;
        location->block = block;

        loadKey(location, key, keytype);
        authenticate(static_cast<unsigned char>(getSectorStartBlock(sector)), key->getKeyStorage(), keytype);
    }

    std::vector<unsigned char> MifareCommands::readSector(int sector, int start_block, const MifareAccessInfo::SectorAccessBits& sab, bool readtrailer)
    {
        std::vector<unsigned char> ret;

        int nbblocks = getNbBlocks(sector);
        if (readtrailer)
        {
            nbblocks += 1;
        }

        for (int i = start_block; i < nbblocks; i++)
        {
            changeBlock(sab, sector, i, false);
            std::vector<unsigned char> data = readBinary(static_cast<unsigned char>(getSectorStartBlock(sector) + i), 16);
			ret.insert(ret.end(), data.begin(), data.end());
        }

        return ret;
    }

    void MifareCommands::writeSector(int sector, int start_block, const std::vector<unsigned char>& buf, const MifareAccessInfo::SectorAccessBits& sab, unsigned char userbyte, MifareAccessInfo::SectorAccessBits* newsab)
    {
        size_t retlen = 0;

        for (int i = start_block; i < getNbBlocks(sector); i++)
        {
            changeBlock(sab, sector, i, true);
            updateBinary(static_cast<unsigned char>(getSectorStartBlock(sector) + i), std::vector<unsigned char>(buf.begin() + retlen, buf.begin() + retlen + 16));
			retlen += 16;
        }

        if (newsab != NULL)
        {
            int writeSector = getMifareChip()->getNbSectors();
            changeKey(getMifareChip()->getMifareProfile()->getKey(writeSector, KT_KEY_A), getMifareChip()->getMifareProfile()->getKey(writeSector, KT_KEY_B), sector, sab, newsab, userbyte);
        }
    }

    void MifareCommands::changeKey(std::shared_ptr<MifareKey> keyA, std::shared_ptr<MifareKey> keyB, unsigned int sector, const MifareAccessInfo::SectorAccessBits& sab, MifareAccessInfo::SectorAccessBits* newsab, unsigned char userbyte)
    {
        std::vector<unsigned char> trailerblock(16, 0x00);

        if (!keyB || keyB->isEmpty())
        {
            memcpy(&trailerblock[0], keyA->getData(), MIFARE_KEY_SIZE);
        }
        else
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
        updateBinary(static_cast<unsigned char>(getSectorStartBlock(sector) + getNbBlocks(sector)), trailerblock);
    }

    std::vector<unsigned char> MifareCommands::readSectors(int start_sector, int stop_sector, int start_block, const MifareAccessInfo::SectorAccessBits& sab)
    {
        if (start_sector > stop_sector)
        {
            THROW_EXCEPTION_WITH_LOG(std::invalid_argument, "Start sector can't be greater than stop sector.");
        }

		std::vector<unsigned char> ret;

        for (int i = start_sector; i <= stop_sector; ++i)
        {
            int startBlockSector = (i == start_sector) ? start_block : 0;
            std::vector<unsigned char> data = readSector(i, startBlockSector, sab);

			ret.insert(ret.end(), data.begin(), data.end());
        }

        return ret;
    }

    void MifareCommands::writeSectors(int start_sector, int stop_sector, int start_block, const std::vector<unsigned char>& buf, const MifareAccessInfo::SectorAccessBits& sab, unsigned char userbyte, MifareAccessInfo::SectorAccessBits* newsab)
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

    unsigned char MifareCommands::getNbBlocks(int sector)
    {
        return ((sector >= 32) ? 15 : 3);
    }

    unsigned char MifareCommands::getSectorStartBlock(int sector)
    {
        unsigned char start_block = 0;
        for (int i = 0; i < sector; ++i)
        {
            start_block += getNbBlocks(i) + 1;
        }

        return start_block;
    }

    std::shared_ptr<MifareChip> MifareCommands::getMifareChip() const
    {
        return std::dynamic_pointer_cast<MifareChip>(getChip());
    }

    bool MifareCommands::writeValueBlock(uint8_t blockno, int32_t input_value, uint8_t backup_blockno)
    {
        int32_t value                   = input_value;
        int32_t reverse                 = value ^ 0xFFFFFFFF;
        uint8_t backup_blockno_reverse  = backup_blockno ^ 0xFF;

        std::vector<uint8_t> buf(16);
        memcpy(&buf[0], &value, 4);
        memcpy(&buf[4], &reverse, 4);
        memcpy(&buf[8], &value, 4);
        memcpy(&buf[12], &backup_blockno, 1);
        memcpy(&buf[13], &backup_blockno_reverse, 1);
        memcpy(&buf[14], &backup_blockno, 1);
        memcpy(&buf[15], &backup_blockno_reverse, 1);
        updateBinary(blockno, buf);

        return true;
    }

    bool MifareCommands::readValueBlock(uint8_t blockno, int32_t &value, uint8_t &backup_block)
    {
        auto buffer = readBinary(blockno, 16);
        EXCEPTION_ASSERT_WITH_LOG(buffer.size() == 16, LibLogicalAccessException,
                                  "Didn't manage to read 16 bytes of data. Has " +
                                      std::to_string(buffer.size()) + " instead.");

        int32_t value0, value1, value_reverse;
        uint8_t backup0, backup1, backup0_reverse, backup1_reverse;

        int idx = 0;
        memcpy(&value0, &buffer[idx], 4);           idx += 4;
        memcpy(&value_reverse, &buffer[idx], 4);    idx += 4;
        memcpy(&value1, &buffer[idx], 4);           idx +=4;
        memcpy(&backup0, &buffer[idx], 1);          idx++;
        memcpy(&backup0_reverse, &buffer[idx], 1);  idx++;
        memcpy(&backup1, &buffer[idx], 1);          idx++;
        memcpy(&backup1_reverse, &buffer[idx], 1);  idx++;

        if (value0 == value1 &&
            (static_cast<int32_t>(value0 ^ 0xFFFFFFFF) == value_reverse) &&
            backup0 == backup1 && (backup0 ^ 0xFF) == backup0_reverse &&
            backup0_reverse == backup1_reverse)
        {
            value = value0;
            backup_block = backup0;
            return true;
        }
        return false;
    }
}
