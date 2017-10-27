/**
 * \file mifarecherrycommands.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Mifare Cherry commands.
 */

#include "../commands/mifarecherrycommands.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>

#include "../pcscreaderprovider.hpp"
#include "mifare/mifarechip.hpp"

namespace logicalaccess
{
MifareCherryCommands::MifareCherryCommands()
    : MifarePCSCCommands(CMD_MIFARECHERRY)
{
}

MifareCherryCommands::MifareCherryCommands(std::string ct)
    : MifarePCSCCommands(ct)
{
}

MifareCherryCommands::~MifareCherryCommands()
{
}

bool MifareCherryCommands::loadKey(unsigned char keyno, MifareKeyType /*keytype*/,
                                   std::shared_ptr<MifareKey> key, bool /*vol*/)
{
    bool r = false;

    // To check on Cherry documentation why key #0 failed.
    if (keyno == 0)
    {
        keyno = 1;
    }
    ByteVector vector_key((unsigned char *)key->getData(),
                          (unsigned char *)key->getData() + key->getLength());
    getPCSCReaderCardAdapter()->sendAPDUCommand(
        0xFF, 0x82, 0x00, keyno, static_cast<unsigned char>(key->getLength()),
        vector_key);
    r = true;

    return r;
}

void MifareCherryCommands::authenticate(unsigned char blockno, unsigned char keyno,
                                        MifareKeyType keytype)
{
    ByteVector command;

    // To check on Cherry documentation why key #0 failed.
    if (keyno == 0)
    {
        keyno = 1;
    }

    command.push_back(0x01);
    command.push_back(0x00);
    command.push_back(blockno);
    command.push_back(static_cast<unsigned char>(keytype));
    command.push_back(keyno);

    getPCSCReaderCardAdapter()->sendAPDUCommand(
        0xFF, 0x86, 0x00, 0x00, static_cast<unsigned char>(command.size()), command);
}
}