//
// Created by xaqq on 6/12/15.
//

#include <logicalaccess/myexception.hpp>
#include "MifarePlusSL0Commands.hpp"
#include "../mifare/mifareaccessinfo.hpp"
#include <cassert>
#include <logicalaccess/logs.hpp>

using namespace logicalaccess;

int MifarePlusSL0Commands::detectSecurityLevel() const
{
    if (probeLevel3())
    {
        LOG(INFOS) << "Card appear to be level 3";
        return 3;
    }
    if (isLevel0())
    {
        return 0;
    }
    return -1;
}

bool MifarePlusSL0Commands::probeLevel3() const
{
    // Attempt to auth with the AES key 0
    ByteVector cmd = {0x70, 0x00, 0x40, 0x01, 0x00};
    try
    {
        getReaderCardAdapter()->sendCommand(cmd);
    }
    catch (const CardException &)
    {
        return false;
    }
    return true;
}

bool MifarePlusSL0Commands::isLevel0() const
{
    // Attempt to write full 0x00 in block 0x01, sector 0x00.
    try
    {
        writePerso(0x00, 0x01, {{0}});
        LOG(INFOS) << "Rewrote sector 0 block 1 with zeroes";
    }
    catch (const CardException &)
    {
        return false;
    }
    return true;
}

bool MifarePlusSL0Commands::writePerso(uint8_t hex_addr_1, uint8_t hex_addr_2,
                                       const std::array<uint8_t, 16> &data) const
{
    ByteVector cmd = {0xA8, hex_addr_2, hex_addr_1};
    cmd.insert(cmd.end(), data.begin(), data.end());

    if (is_trailing_block(hex_addr_1, hex_addr_2) && !validate_access_bytes(data))
    {
        LOG(ERRORS) << "Access conditions bytes are invalid. Refusing to write.";
        return false;
    }

    getReaderCardAdapter()->sendCommand(cmd);

    return true;
}

bool MifarePlusSL0Commands::writePerso(const MifarePlusBlockKeyLocation &location,
                                       const std::array<uint8_t, 16> &data) const
{
    uint16_t loc = static_cast<uint16_t>(location);
    return writePerso(loc >> 8, loc & 0xFF, data);
}

bool MifarePlusSL0Commands::commitPerso() const
{
    ByteVector cmd = {0xAA};
    getReaderCardAdapter()->sendCommand(cmd);

    return true;
}

bool MifarePlusSL0Commands::is_trailing_block(uint8_t hex_addr1, uint8_t hex_addr2)
{
    if (hex_addr1 == 0)
    {
        if (hex_addr2 <= 0x7F)
        {
            if (hex_addr2 % 4 == 3)
                return true;
            return false;
        }
        if (hex_addr2 % 16 == 15)
            return true;
        return false;
    }
    return false;
}

bool MifarePlusSL0Commands::validate_access_bytes(const std::array<uint8_t, 16> &data)
{
    ByteVector access_bytes = {{data[6], data[7], data[8]}};
    MifareAccessInfo::SectorAccessBits sab;
    return sab.fromArray(access_bytes);
}

bool MifarePlusSL0Commands::is4K() const
{
    if (isLevel0())
    {
        // we try to write in a the last block that is not a sector trailer.
        try
        {
            if (writePerso(0x00, 0xFE, {{0}}))
            {
                return true;
            }
        }
        catch (...)
        {
        }
        return false;
    }

    if (probeLevel3())
    {
        // We don't know what do to, lets just die.
        assert(0);
    }
    assert(0);
    return false;
}
