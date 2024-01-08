/**
* \file topazomnikeyxx27commands.cpp
* \author Maxime C. <maxime@leosac.com>
* \brief Topaz Omnikey XX27 PC/SC commands.
*/

#include <logicalaccess/plugins/readers/pcsc/commands/topazomnikeyxx27commands.hpp>

#include <iostream>
#include <iomanip>
#include <sstream>

#include <logicalaccess/plugins/readers/pcsc/pcscreaderprovider.hpp>
#include <logicalaccess/plugins/cards/topaz/topazchip.hpp>

namespace logicalaccess
{
TopazOmnikeyXX27Commands::TopazOmnikeyXX27Commands()
    : TopazCommands(CMD_TOPAZOKXX27)
{
}

TopazOmnikeyXX27Commands::TopazOmnikeyXX27Commands(std::string ct)
    : TopazCommands(ct)
{
}

TopazOmnikeyXX27Commands::~TopazOmnikeyXX27Commands()
{
}

ByteVector TopazOmnikeyXX27Commands::readPage(int page)
{
    return getPCSCReaderCardAdapter()->sendAPDUCommand(
        0xFF, 0xB0, 0x00, static_cast<unsigned char>(page), 8).getData();
}

void TopazOmnikeyXX27Commands::writePage(int page, const ByteVector &buf)
{
    if (buf.size() > 8)
    {
        THROW_EXCEPTION_WITH_LOG(std::invalid_argument, "Bad buffer parameter.");
    }

    getPCSCReaderCardAdapter()->sendAPDUCommand(
        0xFF, 0xD6, 0x00, static_cast<unsigned char>(page),
        static_cast<unsigned char>(buf.size()), buf);
}
}