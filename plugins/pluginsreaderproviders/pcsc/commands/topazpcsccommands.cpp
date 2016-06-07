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
#include "topazchip.hpp"

namespace logicalaccess
{
    TopazPCSCCommands::TopazPCSCCommands()
        : TopazCommands()
    {
    }

    TopazPCSCCommands::~TopazPCSCCommands()
    {
    }

    std::vector<unsigned char> TopazPCSCCommands::readPage(int page)
    {
        std::vector<unsigned char> result = getPCSCReaderCardAdapter()->sendAPDUCommand(0xFF, 0xB0, 0x00, static_cast<unsigned char>((page) * 8), 8);

        return std::vector<unsigned char>(result.begin(), result.end() - 2);
    }

    void TopazPCSCCommands::writePage(int page, const std::vector<unsigned char>& buf)
    {
        if (buf.size() > 8)
        {
            THROW_EXCEPTION_WITH_LOG(std::invalid_argument, "Bad buffer parameter.");
        }

        getPCSCReaderCardAdapter()->sendAPDUCommand(0xFF, 0xD6, 0x00, static_cast<unsigned char>((page) * 8), static_cast<unsigned char>(buf.size()), buf);
    }
}