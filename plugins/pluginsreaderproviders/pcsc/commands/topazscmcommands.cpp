/**
* \file topazscmcommands.cpp
* \author Maxime C. <maxime-dev@islog.com>
* \brief Topaz SCM PC/SC commands.
*/

#include "../commands/topazscmcommands.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>

#include "../pcscreaderprovider.hpp"
#include "topazchip.hpp"

namespace logicalaccess
{
    TopazSCMCommands::TopazSCMCommands()
        : TopazCommands()
    {
    }

    TopazSCMCommands::~TopazSCMCommands()
    {
    }

    std::vector<unsigned char> TopazSCMCommands::readPage(int page)
    {
        std::vector<unsigned char> data;
        for (unsigned char i = 0; i < 8; ++i)
        {
            unsigned char address = page << 3 | i;
            std::vector<unsigned char> result = getPCSCReaderCardAdapter()->sendAPDUCommand(0xFF, 0x54, 0x00, address, 0x00);
            data.push_back(result[0]);
        }

        return data;
    }

    void TopazSCMCommands::writePage(int page, const std::vector<unsigned char>& buf)
    {
        if (buf.size() > 8)
        {
            THROW_EXCEPTION_WITH_LOG(std::invalid_argument, "Bad buffer parameter.");
        }

        for (unsigned char i = 0; i < 8; ++i)
        {
            unsigned char address = page << 3 | i;
            getPCSCReaderCardAdapter()->sendAPDUCommand(0xFF, 0x56, 0x00, address, 0x01, buf[i]);
        }
    }
}