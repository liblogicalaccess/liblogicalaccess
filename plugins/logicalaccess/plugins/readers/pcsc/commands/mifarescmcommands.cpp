/**
 * \file mifarescmcommands.cpp
 * \author Maxime C. <maxime@leosac.com>
 * \brief Mifare SCM commands.
 */

#include <logicalaccess/plugins/readers/pcsc/commands/mifarescmcommands.hpp>

#include <iostream>
#include <iomanip>
#include <sstream>

#include <logicalaccess/plugins/readers/pcsc/pcscreaderprovider.hpp>
#include <logicalaccess/plugins/cards/mifare/mifarechip.hpp>

namespace logicalaccess
{
MifareSCMCommands::MifareSCMCommands()
    : MifarePCSCCommands(CMD_MIFARESCM)
{
}

MifareSCMCommands::MifareSCMCommands(std::string ct)
    : MifarePCSCCommands(ct)
{
}


MifareSCMCommands::~MifareSCMCommands()
{
}

bool MifareSCMCommands::loadKey(unsigned char keyno, MifareKeyType keytype,
                                std::shared_ptr<MifareKey> key, bool vol)
{
    bool r = false;

    ByteVector vector_key((unsigned char *)key->getData(),
                          (unsigned char *)key->getData() + key->getLength());

    auto result = getPCSCReaderCardAdapter()->sendAPDUCommand(
        0xFF, 0x82, 0x00, static_cast<unsigned char>(keytype),
        static_cast<unsigned char>(vector_key.size()), vector_key);

    if (!vol && result.getSW1() == 0x63 && result.getSW2() == 0x86)
    {
        if (keyno == 0)
        {
            r = loadKey(keyno, keytype, key, true);
        }
    }
    else
    {
        r = true;
    }

    return r;
}

void MifareSCMCommands::increment(unsigned char blockno, uint32_t value)
{
    // Somehow the documentation is invalid, and increment and decrement
    // operation code are reversed (SCL011).
    ByteVector buf;
    buf.push_back(0xC1);
    buf.push_back(blockno);
    buf.push_back(static_cast<unsigned char>(value & 0xff));
    buf.push_back(static_cast<unsigned char>((value >> 8) & 0xff));
    buf.push_back(static_cast<unsigned char>((value >> 16) & 0xff));
    buf.push_back(static_cast<unsigned char>((value >> 24) & 0xff));

    getPCSCReaderCardAdapter()->sendAPDUCommand(
        0xFF, 0xF0, 0x00, blockno, static_cast<unsigned char>(buf.size()), buf);
}

void MifareSCMCommands::decrement(unsigned char blockno, uint32_t value)
{
    // Somehow the documentation is invalid, and increment and decrement
    // operation code are reversed (SCL011).
    ByteVector buf;
    buf.push_back(0xC0);
    buf.push_back(blockno);
    buf.push_back(static_cast<unsigned char>(value & 0xff));
    buf.push_back(static_cast<unsigned char>((value >> 8) & 0xff));
    buf.push_back(static_cast<unsigned char>((value >> 16) & 0xff));
    buf.push_back(static_cast<unsigned char>((value >> 24) & 0xff));

    getPCSCReaderCardAdapter()->sendAPDUCommand(
        0xFF, 0xF0, 0x00, blockno, static_cast<unsigned char>(buf.size()), buf);
}
}