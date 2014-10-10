/**
 * \file mifareultralightcacsacrcommands.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Mifare Ultralight C - ACS ACR.
 */

#include "../commands/mifareultralightcacsacrcommands.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>

#include "../pcscreaderprovider.hpp"
#include "logicalaccess/cards/computermemorykeystorage.hpp"
#include "logicalaccess/cards/readermemorykeystorage.hpp"
#include "logicalaccess/cards/samkeystorage.hpp"

namespace logicalaccess
{
    MifareUltralightCACSACRCommands::MifareUltralightCACSACRCommands()
        : MifareUltralightCPCSCCommands()
    {
    }

    MifareUltralightCACSACRCommands::~MifareUltralightCACSACRCommands()
    {
    }

    std::vector<unsigned char> MifareUltralightCACSACRCommands::sendGenericCommand(const std::vector<unsigned char>& data)
    {
        return getPCSCReaderCardAdapter()->sendAPDUCommand(0xFF, 0x00, 0x00, 0x00, static_cast<unsigned char>(data.size()), data);
    }
}