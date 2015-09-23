/**
 * \file mifarescmcommands.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Mifare SCM commands.
 */

#include "../commands/mifareomnikeyxx27commands.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>

#include "../pcscreaderprovider.hpp"
#include "mifarechip.hpp"

namespace logicalaccess
{
    MifareOmnikeyXX27Commands::MifareOmnikeyXX27Commands()
        : MifarePCSCCommands()
    {
    }

    MifareOmnikeyXX27Commands::~MifareOmnikeyXX27Commands()
    {
    }

    void MifareOmnikeyXX27Commands::authenticate(unsigned char blockno, unsigned char keyno, MifareKeyType keytype)
    {
        TRACE(blockno, keyno, keytype)
        std::vector<unsigned char> command;

        command.push_back(0x01);
        command.push_back(0x00);
        command.push_back(blockno);
        command.push_back(static_cast<unsigned char>(keytype));
        command.push_back(keyno);

        getPCSCReaderCardAdapter()->sendAPDUCommand(0xFF, 0x86, 0x00, 0x00, static_cast<unsigned char>(command.size()), command);
    }

}
