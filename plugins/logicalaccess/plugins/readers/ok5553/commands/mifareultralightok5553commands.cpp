/**
 * \file mifareultralightok5553commands.cpp
 * \author Maxime C. <maxime@leosac.com>
 * \brief Mifare Ultralight commands for OK5553 readers.
 */

#include <logicalaccess/plugins/readers/ok5553/commands/mifareultralightok5553commands.hpp>

#include <iostream>
#include <iomanip>
#include <sstream>

#include <logicalaccess/plugins/cards/mifare/mifarechip.hpp>

namespace logicalaccess
{
MifareUltralightOK5553Commands::MifareUltralightOK5553Commands()
    : MifareUltralightCommands(CMD_MIFAREULTRALIGHTOK5553)
{
}

MifareUltralightOK5553Commands::MifareUltralightOK5553Commands(std::string ct)
    : MifareUltralightCommands(ct)
{
}

MifareUltralightOK5553Commands::~MifareUltralightOK5553Commands()
{
}

ByteVector MifareUltralightOK5553Commands::readPage(int page)
{
    ByteVector command;
    char buffer[3];
    command.push_back(static_cast<unsigned char>('r'));
    command.push_back(static_cast<unsigned char>('b'));
    sprintf(buffer, "%.2X", page);
    command.push_back(static_cast<unsigned char>(buffer[0]));
    command.push_back(static_cast<unsigned char>(buffer[1]));
    ByteVector answer = getOK5553ReaderCardAdapter()->sendCommand(command);
    return OK5553ReaderUnit::asciiToHex(answer);
}

void MifareUltralightOK5553Commands::writePage(int page, const ByteVector &buf)
{
    ByteVector command;
    char buffer[3];
    command.push_back(static_cast<unsigned char>('w'));
    command.push_back(static_cast<unsigned char>('b'));
    sprintf(buffer, "%.2X", page);
    command.push_back(static_cast<unsigned char>(buffer[0]));
    command.push_back(static_cast<unsigned char>(buffer[1]));
    for (size_t i = 0; i < buf.size(); i++)
    {
        sprintf(buffer, "%.2X", buf[i]);
        command.push_back(static_cast<unsigned char>(buffer[0]));
        command.push_back(static_cast<unsigned char>(buffer[1]));
    }
    getOK5553ReaderCardAdapter()->sendCommand(command);
}
}