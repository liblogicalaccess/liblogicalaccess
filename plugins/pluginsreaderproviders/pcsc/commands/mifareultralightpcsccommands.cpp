/**
 * \file mifareultralightpcsccommands.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Mifare Ultralight PC/SC commands.
 */

#include "../commands/mifareultralightpcsccommands.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>

#include "../pcscreaderprovider.hpp"
#include "mifareultralight/mifareultralightchip.hpp"
#include "logicalaccess/cards/computermemorykeystorage.hpp"
#include "logicalaccess/cards/readermemorykeystorage.hpp"
#include "logicalaccess/cards/samkeystorage.hpp"

namespace logicalaccess
{
    MifareUltralightPCSCCommands::MifareUltralightPCSCCommands()
        : MifareUltralightCommands()
    {
    }

    MifareUltralightPCSCCommands::~MifareUltralightPCSCCommands()
    {
    }

    std::vector<unsigned char> MifareUltralightPCSCCommands::readPage(int page)
    {
        std::vector<unsigned char> result = getPCSCReaderCardAdapter()->sendAPDUCommand(0xFF, 0xB0, 0x00, static_cast<unsigned char>(page), 16);

		return std::vector<unsigned char>(result.begin(), result.end() - 2);
    }

    void MifareUltralightPCSCCommands::writePage(int page, const std::vector<unsigned char>& buf)
    {
        if (buf.size() > 16)
        {
            THROW_EXCEPTION_WITH_LOG(std::invalid_argument, "Bad buffer parameter.");
        }

        getPCSCReaderCardAdapter()->sendAPDUCommand(0xFF, 0xD6, 0x00, static_cast<unsigned char>(page), static_cast<unsigned char>(buf.size()), buf);
    }
}