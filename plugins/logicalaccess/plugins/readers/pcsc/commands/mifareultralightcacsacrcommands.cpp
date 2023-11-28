/**
 * \file mifareultralightcacsacrcommands.cpp
 * \author Maxime C. <maxime@leosac.com>
 * \brief Mifare Ultralight C - ACS ACR.
 */

#include <logicalaccess/plugins/readers/pcsc/commands/mifareultralightcacsacrcommands.hpp>

#include <iostream>
#include <iomanip>
#include <sstream>

#include <logicalaccess/plugins/readers/pcsc/pcscreaderprovider.hpp>
#include <logicalaccess/cards/computermemorykeystorage.hpp>
#include <logicalaccess/cards/readermemorykeystorage.hpp>
#include <logicalaccess/cards/samkeystorage.hpp>

namespace logicalaccess
{
MifareUltralightCACSACRCommands::MifareUltralightCACSACRCommands()
    : MifareUltralightCPCSCCommands(CMD_MIFAREULTRALIGHTCACSACR)
{
}

MifareUltralightCACSACRCommands::MifareUltralightCACSACRCommands(std::string ct)
    : MifareUltralightCPCSCCommands(ct)
{
}

MifareUltralightCACSACRCommands::~MifareUltralightCACSACRCommands()
{
}

ISO7816Response MifareUltralightCACSACRCommands::sendGenericCommand(const ByteVector &data)
{
    return ISO7816Response(getPCSCReaderCardAdapter()->sendAPDUCommand(
        0xFF, 0x00, 0x00, 0x00, static_cast<unsigned char>(data.size()), data).getData());
}
}