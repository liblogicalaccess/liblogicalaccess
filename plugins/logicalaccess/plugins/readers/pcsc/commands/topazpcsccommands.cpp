/**
* \file topazpcsccommands.cpp
* \author Maxime C. <maxime@leosac.com>
* \brief Topaz PC/SC commands.
*/

#include <logicalaccess/plugins/readers/pcsc/commands/topazpcsccommands.hpp>

#include <iostream>
#include <iomanip>
#include <sstream>

#include <logicalaccess/plugins/readers/pcsc/pcscreaderprovider.hpp>
#include <logicalaccess/plugins/cards/topaz/topazchip.hpp>

namespace logicalaccess
{
TopazPCSCCommands::TopazPCSCCommands()
    : TopazCommands(CMD_TOPAZPCSC)
{
}

TopazPCSCCommands::TopazPCSCCommands(std::string ct)
    : TopazCommands(ct)
{
}

TopazPCSCCommands::~TopazPCSCCommands()
{
}

ByteVector TopazPCSCCommands::readPage(int page)
{
    return getPCSCReaderCardAdapter()
        ->sendAPDUCommand(0xFF, 0xB0, 0x00, static_cast<unsigned char>((page)*8), 8)
        .getData();
}

void TopazPCSCCommands::writePage(int page, const ByteVector &buf)
{
    if (buf.size() > 8)
    {
        THROW_EXCEPTION_WITH_LOG(std::invalid_argument, "Bad buffer parameter.");
    }

    getPCSCReaderCardAdapter()->sendAPDUCommand(
        0xFF, 0xD6, 0x00, static_cast<unsigned char>((page)*8),
        static_cast<unsigned char>(buf.size()), buf);
}
}