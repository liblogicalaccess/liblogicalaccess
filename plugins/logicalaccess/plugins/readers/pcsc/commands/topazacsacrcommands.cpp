/**
* \file topazacsacrcommands.cpp
* \author Maxime C. <maxime@leosac.com>
* \brief Topaz ACS ACR PC/SC commands.
*/

#include <logicalaccess/plugins/readers/pcsc/commands/topazacsacrcommands.hpp>

#include <iostream>
#include <iomanip>
#include <sstream>

#include <logicalaccess/plugins/readers/pcsc/pcscreaderprovider.hpp>
#include <logicalaccess/plugins/cards/topaz/topazchip.hpp>

namespace logicalaccess
{
TopazACSACRCommands::TopazACSACRCommands()
    : TopazCommands(CMD_TOPAZACSACR)
{
}

TopazACSACRCommands::TopazACSACRCommands(std::string ct)
    : TopazCommands(ct)
{
}

TopazACSACRCommands::~TopazACSACRCommands()
{
}

ByteVector TopazACSACRCommands::readPage(int page)
{
    ByteVector data;
    ByteVector cmd;
    // Pseudo APDU, Data Exchange command
    cmd.push_back(0xD4);
    cmd.push_back(0x40);
    cmd.push_back(0x01);
    cmd.push_back(0x01); // Chip Command
    cmd.push_back(0x00); // Byte to read
    for (unsigned char i = 0; i < 8; ++i)
    {
        cmd[4]            = static_cast<unsigned char>(((page)*8) + i);
        ByteVector result = getPCSCReaderCardAdapter()->sendAPDUCommand(
            0xFF, 0x00, 0x00, 0x00, static_cast<unsigned char>(cmd.size()), cmd).getData();
        // D5 41 00 xx
        if (result.size() == 4)
        {
            data.push_back(result[3]);
        }
    }

    return data;
}

void TopazACSACRCommands::writePage(int page, const ByteVector &buf)
{
    if (buf.size() > 8)
    {
        THROW_EXCEPTION_WITH_LOG(std::invalid_argument, "Bad buffer parameter.");
    }

    ByteVector cmd;
    // Pseudo APDU
    cmd.push_back(0xD4);
    cmd.push_back(0x40);
    cmd.push_back(0x01);
    cmd.push_back(0x53); // Card Command
    cmd.push_back(0x00); // Byte to write
    cmd.push_back(0x00); // Byte data
    for (unsigned int i = 0; i < buf.size(); ++i)
    {
        cmd[4] = static_cast<unsigned char>(((page)*8) + i);
        cmd[5] = buf[i];
        getPCSCReaderCardAdapter()->sendAPDUCommand(
            0xFF, 0x00, 0x00, 0x00, static_cast<unsigned char>(cmd.size()), cmd);
    }
}
}