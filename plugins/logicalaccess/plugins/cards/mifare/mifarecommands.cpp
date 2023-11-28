/**
 * \file mifarecommands.cpp
 * \author Julien K. <julien-dev@islog.com>, Maxime C. <maxime@leosac.com>
 * \brief Mifare commands.
 */

#include <cstring>
#include <logicalaccess/plugins/llacommon/logs.hpp>
#include <cassert>
#include <algorithm>
#include <logicalaccess/plugins/cards/mifare/mifarecommands.hpp>
#include <logicalaccess/plugins/cards/mifare/mifarechip.hpp>
#include <logicalaccess/plugins/cards/mifare/mifarelocation.hpp>
#include <logicalaccess/myexception.hpp>

#define PREFIX_PATTERN 0xE3
#define POLYNOM_PATTERN 0x1D

namespace logicalaccess
{
void MifareCommands::authenticate(std::shared_ptr<Location> location,
                                  std::shared_ptr<AccessInfo> ai, bool write)
{
    EXCEPTION_ASSERT_WITH_LOG(location, std::invalid_argument,
                              "location cannot be null.");
    EXCEPTION_ASSERT_WITH_LOG(ai, std::invalid_argument, "ai cannot be null.");

    std::shared_ptr<MifareLocation> mLocation =
        std::dynamic_pointer_cast<MifareLocation>(location);
    std::shared_ptr<MifareAccessInfo> mAi =
        std::dynamic_pointer_cast<MifareAccessInfo>(ai);

    EXCEPTION_ASSERT_WITH_LOG(mLocation, std::invalid_argument,
                              "location must be a MifareLocation.");
    EXCEPTION_ASSERT_WITH_LOG(mAi, std::invalid_argument,
                              "ai must be a MifareAccessInfo.");

    MifareKeyType keytype =
        getKeyType(mAi->sab, mLocation->sector, mLocation->block, write);
    std::shared_ptr<MifareKey> key = keytype == KT_KEY_A ? mAi->keyA : mAi->keyB;
    EXCEPTION_ASSERT_WITH_LOG(key, std::invalid_argument,
                              "ai doesn't contains target key");

    loadKey(location, keytype, key);
    authenticate(static_cast<unsigned char>(getSectorStartBlock(mLocation->sector)),
                 key->getKeyStorage(), keytype);
}

unsigned int MifareCommands::getSectorFromMAD(long aid,
                                              std::shared_ptr<MifareKey> madKeyA)
{
    unsigned int sector = static_cast<unsigned int>(-1);
    MifareAccessInfo::SectorAccessBits sab;
    sab.setTransportConfiguration();

    ByteVector madbuf = readSector(0, 1, madKeyA, std::shared_ptr<MifareKey>(), sab);
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

    if ((sector == static_cast<unsigned int>(-1)) &&
        getChip()->getCardType() == "Mifare4K")
    {
        madbuf = readSector(16, 0, madKeyA, std::shared_ptr<MifareKey>(), sab);
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

void MifareCommands::setSectorToMAD(long aid, unsigned int sector,
                                    std::shared_ptr<MifareKey> madKeyA,
                                    std::shared_ptr<MifareKey> madKeyB)
{
    MifareAccessInfo::SectorAccessBits sab;
    if (madKeyB->isEmpty())
    {
        sab.setTransportConfiguration();
    }
    else
    {
        sab.setAReadBWriteConfiguration();
    }

    if (sector < 16)
    {
        if (sector == 0)
        {
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                     "Can't make reference to the MAD itself.");
        }

        ByteVector madbuf = readSector(0, 1, madKeyA, madKeyB, sab);
        if (!madbuf.size())
        {
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Can't read the MAD.");
        }

        madbuf[(sector * 2)]   = aid & 0xff;
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

        writeSector(0, 1, madbuf, madKeyA, madKeyB, sab);
    }
    else
    {
        if (sector == 16)
        {
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                     "Can't make reference to the MAD2 itself.");
        }

        ByteVector madbuf = readSector(16, 0, madKeyA, madKeyB, sab);
        if (!madbuf.size())
        {
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Can't read the MAD2.");
        }

        sector -= 16;
        madbuf[sector * 2]       = (aid & 0xff00) >> 8;
        madbuf[(sector * 2) + 1] = aid & 0xff;
        madbuf[0]                = calculateMADCrc(&madbuf[0], madbuf.size());

        writeSector(16, 0, madbuf, madKeyA, madKeyB, sab);
    }
}

unsigned char MifareCommands::calculateMADCrc(const unsigned char *madbuf,
                                              size_t madbuflen)
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

unsigned int MifareCommands::findReferencedSector(long aid, unsigned char *madbuf,
                                                  size_t madbuflen)
{
    unsigned int sector = static_cast<unsigned int>(-1);

    for (unsigned int i = 1; i < madbuflen && (sector == static_cast<unsigned int>(-1));
         ++i)
    {
        long paid = 0;
        paid      = madbuf[(i * 2)] | (madbuf[i * 2 + 1] << 8);

        if (paid == aid)
        {
            sector = i;
        }
    }

    return sector;
}

MifareKeyType MifareCommands::getKeyType(const MifareAccessInfo::SectorAccessBits &sab,
                                         int sector, int block, bool write)
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

    /*
    ** We need to add a "never" key type for the MifareKeyType enum
    */

    if (virtualblock == 3)
    {
        rkt = KT_KEY_A;
        wkt = KT_KEY_A;
        if (sab.d_sector_trailer_access_bits.c1 && !sab.d_sector_trailer_access_bits.c2 &&
            !sab.d_sector_trailer_access_bits.c3)
        {
            wkt = KT_KEY_B;
        }
        if (!sab.d_sector_trailer_access_bits.c1 && sab.d_sector_trailer_access_bits.c2 &&
            sab.d_sector_trailer_access_bits.c3)
        {
            wkt = KT_KEY_B;
        }
    }
    else
    {
        if (sab.d_data_blocks_access_bits[virtualblock].c1 &&
            sab.d_data_blocks_access_bits[virtualblock].c2 &&
            sab.d_data_blocks_access_bits[virtualblock].c3)
        {
            // Never read/write access
        }
        else if (!sab.d_data_blocks_access_bits[virtualblock].c3)
        {
            rkt = KT_KEY_A;
            if (!sab.d_data_blocks_access_bits[virtualblock].c1 &&
                !sab.d_data_blocks_access_bits[virtualblock].c2)
            {
                wkt = KT_KEY_A;
            }
            else if (sab.d_data_blocks_access_bits[virtualblock].c1 &&
                     !sab.d_data_blocks_access_bits[virtualblock].c2)
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
            if (!sab.d_data_blocks_access_bits[virtualblock].c1 &&
                sab.d_data_blocks_access_bits[virtualblock].c2)
            {
                wkt = KT_KEY_B;
            }
            else
            {
                // Never write access
            }
        }
    }
    return (write) ? wkt : rkt;
}

void MifareCommands::authenticate(MifareKeyType keytype, std::shared_ptr<MifareKey> key,
                                  int sector, int block, bool /*write*/)
{
    std::shared_ptr<MifareLocation> location(new MifareLocation());
    location->sector = sector;
    location->block  = block;

    if (!key)
    {
        key.reset(new MifareKey("ff ff ff ff ff ff"));
    }

    loadKey(location, keytype, key);
    authenticate(static_cast<unsigned char>(getSectorStartBlock(sector)),
                 key->getKeyStorage(), keytype);
}

ByteVector MifareCommands::readSector(int sector, int start_block,
                                      std::shared_ptr<MifareKey> keyA,
                                      std::shared_ptr<MifareKey> keyB,
                                      const MifareAccessInfo::SectorAccessBits &sab,
                                      bool readtrailer)
{
    ByteVector ret;

    int nbblocks = getNbBlocks(sector);
    if (readtrailer)
    {
        nbblocks += 1;
    }
    MifareKeyType pkeytype;
    for (int i = start_block; i < nbblocks; i++)
    {
        const MifareKeyType keytype = getKeyType(sab, sector, i, false);
        if (i == start_block || keytype != pkeytype)
        {
            authenticate(keytype, keytype == KT_KEY_A ? keyA : keyB, sector, i, false);
            pkeytype = keytype;
        }
        ByteVector data =
            readBinary(static_cast<unsigned char>(getSectorStartBlock(sector) + i), 16);
        ret.insert(ret.end(), data.begin(), data.end());
    }

    return ret;
}

void MifareCommands::writeSector(
    int sector, int start_block, const ByteVector &buf, std::shared_ptr<MifareKey> keyA,
    std::shared_ptr<MifareKey> keyB, const MifareAccessInfo::SectorAccessBits &sab,
    unsigned char userbyte, MifareAccessInfo::SectorAccessBits *newsab,
    std::shared_ptr<MifareKey> newkeyA, std::shared_ptr<MifareKey> newkeyB)
{
    size_t retlen  = 0;
    ByteVector tmp = buf;
    MifareKeyType keytype, pkeytype = KT_KEY_A;

    for (int i = start_block; i < getNbBlocks(sector); i++)
    {
        if (tmp.size() < retlen + 16)
        {
            ByteVector r = readSector(sector, i, keyA, keyB, sab);
            tmp.insert(tmp.end(), r.begin() + tmp.size() % 16, r.end());
        }
        keytype = getKeyType(sab, sector, i, true);
        if (i == start_block || keytype != pkeytype)
        {
            authenticate(keytype, keytype == KT_KEY_A ? keyA : keyB, sector, i, true);
            pkeytype = keytype;
        }
        updateBinary(static_cast<unsigned char>(getSectorStartBlock(sector) + i),
                     ByteVector(tmp.begin() + retlen, tmp.begin() + retlen + 16));
        retlen += 16;
    }

    if (newsab != nullptr)
    {
        keytype = getKeyType(sab, sector, getNbBlocks(sector), true);
        if (keytype != pkeytype)
        {
            authenticate(keytype, keytype == KT_KEY_A ? keyA : keyB, sector,
                         getNbBlocks(sector), true);
        }
        changeKeys(newkeyA, newkeyB, sector, newsab, userbyte);
    }
}

void MifareCommands::changeKeys(MifareKeyType keytype, std::shared_ptr<MifareKey> key,
                                std::shared_ptr<MifareKey> newkeyA,
                                std::shared_ptr<MifareKey> newkeyB, unsigned int sector,
                                MifareAccessInfo::SectorAccessBits *newsab,
                                unsigned char userbyte)
{
    authenticate(keytype, key, sector, getNbBlocks(sector), true);
    changeKeys(newkeyA, newkeyB, sector, newsab, userbyte);
}

void MifareCommands::changeKeys(std::shared_ptr<MifareKey> newkeyA,
                                std::shared_ptr<MifareKey> newkeyB, unsigned int sector,
                                MifareAccessInfo::SectorAccessBits *newsab,
                                unsigned char userbyte)
{
    ByteVector trailerblock(16, 0x00);

    if (!newkeyA)
        newkeyA = std::make_shared<MifareKey>();

    if (!newkeyB || newkeyB->isEmpty())
    {
        memcpy(&trailerblock[0], newkeyA->getData(), MIFARE_KEY_SIZE);
    }
    else
    {
        memcpy(&trailerblock[0], newkeyA->getData(), MIFARE_KEY_SIZE);
        memcpy(&trailerblock[10], newkeyB->getData(), MIFARE_KEY_SIZE);
    }

    auto tmp = (*newsab).toArray();
    std::copy_n(tmp.begin(), 3, trailerblock.begin() + MIFARE_KEY_SIZE);
    if (tmp.size() != 3)
    {
        THROW_EXCEPTION_WITH_LOG(CardException, EXCEPTION_MSG_CHANGEKEY);
    }

    trailerblock[MIFARE_KEY_SIZE + 3] = userbyte;

    updateBinary(
        static_cast<unsigned char>(getSectorStartBlock(sector) + getNbBlocks(sector)),
        trailerblock);
}

ByteVector MifareCommands::readSectors(int start_sector, int stop_sector, int start_block,
                                       std::shared_ptr<MifareKey> keyA,
                                       std::shared_ptr<MifareKey> keyB,
                                       const MifareAccessInfo::SectorAccessBits &sab)
{
    if (start_sector > stop_sector)
    {
        THROW_EXCEPTION_WITH_LOG(std::invalid_argument,
                                 "Start sector can't be greater than stop sector.");
    }

    ByteVector ret;
    for (int i = start_sector; i <= stop_sector; ++i)
    {
        int startBlockSector = (i == start_sector) ? start_block : 0;
        ByteVector data      = readSector(i, startBlockSector, keyA, keyB, sab);

        ret.insert(ret.end(), data.begin(), data.end());
    }

    return ret;
}

void MifareCommands::writeSectors(int start_sector, int stop_sector, int start_block,
                                  const ByteVector &buf, std::shared_ptr<MifareKey> keyA,
                                  std::shared_ptr<MifareKey> keyB,
                                  const MifareAccessInfo::SectorAccessBits &sab,
                                  unsigned char userbyte,
                                  MifareAccessInfo::SectorAccessBits *newsab,
                                  std::shared_ptr<MifareKey> newkeyA,
                                  std::shared_ptr<MifareKey> newkeyB)
{
    if (start_sector > stop_sector)
    {
        THROW_EXCEPTION_WITH_LOG(std::invalid_argument,
                                 "Start sector can't be greater than stop sector.");
    }

    size_t offset = 0;
    for (int i = start_sector; i <= stop_sector; ++i)
    {
        int startBlockSector = (i == start_sector) ? start_block : 0;
        ByteVector tmp(buf.begin() + offset,
                       buf.begin() + offset + (getNbBlocks(i) - startBlockSector) * 16);
        writeSector(i, startBlockSector, tmp, keyA, keyB, sab, userbyte, newsab, newkeyA,
                    newkeyB);
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

bool MifareCommands::writeValueBlock(uint8_t blockno, int32_t input_value,
                                     uint8_t backup_blockno)
{
    int32_t value                  = input_value;
    int32_t reverse                = value ^ 0xFFFFFFFF;
    uint8_t backup_blockno_reverse = backup_blockno ^ 0xFF;

    ByteVector buf(16);
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

bool MifareCommands::readValueBlock(uint8_t blockno, int32_t &value,
                                    uint8_t &backup_block)
{
    auto buffer = readBinary(blockno, 16);
    EXCEPTION_ASSERT_WITH_LOG(buffer.size() == 16, LibLogicalAccessException,
                              "Didn't manage to read 16 bytes of data.")

    int32_t value0, value1, value_reverse;
    uint8_t backup0, backup1, backup0_reverse, backup1_reverse;

    size_t idx = 0;
    memcpy(&value0, &buffer[idx], 4);
    idx += 4;
    memcpy(&value_reverse, &buffer[idx], 4);
    idx += 4;
    memcpy(&value1, &buffer[idx], 4);
    idx += 4;
    memcpy(&backup0, &buffer[idx], 1);
    idx++;
    memcpy(&backup0_reverse, &buffer[idx], 1);
    idx++;
    memcpy(&backup1, &buffer[idx], 1);
    idx++;
    memcpy(&backup1_reverse, &buffer[idx], 1);

    if (value0 == value1 &&
        (static_cast<int32_t>(value0 ^ 0xFFFFFFFF) == value_reverse) &&
        backup0 == backup1 && (backup0 ^ 0xFF) == backup0_reverse &&
        backup0_reverse == backup1_reverse)
    {
        value        = value0;
        backup_block = backup0;
        return true;
    }
    return false;
}
}
