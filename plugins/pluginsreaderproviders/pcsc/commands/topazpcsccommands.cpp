/**
* \file topazpcsccommands.cpp
* \author Maxime C. <maxime-dev@islog.com>
* \brief Topaz PC/SC commands.
*/

#include "../commands/topazpcsccommands.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>

#include "../pcscreaderprovider.hpp"
#include "topaz/topazchip.hpp"

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
        ByteVector result = getPCSCReaderCardAdapter()->sendAPDUCommand(0xFF, 0xB0, 0x00, static_cast<unsigned char>((page) * 8), 8);

        return ByteVector(result.begin(), result.end() - 2);
    }

    void TopazPCSCCommands::writePage(int page, const ByteVector& buf)
    {
        if (buf.size() > 8)
        {
            THROW_EXCEPTION_WITH_LOG(std::invalid_argument, "Bad buffer parameter.");
        }

        getPCSCReaderCardAdapter()->sendAPDUCommand(0xFF, 0xD6, 0x00, static_cast<unsigned char>((page) * 8), static_cast<unsigned char>(buf.size()), buf);
    }
}