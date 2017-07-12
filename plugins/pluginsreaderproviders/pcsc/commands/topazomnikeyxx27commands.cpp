/**
* \file topazomnikeyxx27commands.cpp
* \author Maxime C. <maxime-dev@islog.com>
* \brief Topaz Omnikey XX27 PC/SC commands.
*/

#include "../commands/topazomnikeyxx27commands.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>

#include "../pcscreaderprovider.hpp"
#include "topaz/topazchip.hpp"

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

    std::vector<unsigned char> TopazOmnikeyXX27Commands::readPage(int page)
    {
        std::vector<unsigned char> result = getPCSCReaderCardAdapter()->sendAPDUCommand(0xFF, 0xB0, 0x00, static_cast<unsigned char>(page), 8);

        return std::vector<unsigned char>(result.begin(), result.end() - 2);
    }

    void TopazOmnikeyXX27Commands::writePage(int page, const std::vector<unsigned char>& buf)
    {
        if (buf.size() > 8)
        {
            THROW_EXCEPTION_WITH_LOG(std::invalid_argument, "Bad buffer parameter.");
        }

        getPCSCReaderCardAdapter()->sendAPDUCommand(0xFF, 0xD6, 0x00, static_cast<unsigned char>(page), static_cast<unsigned char>(buf.size()), buf);
    }
}