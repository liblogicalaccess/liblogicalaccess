/**
 * \file mifarespringcardcommands.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Mifare SpringCard commands.
 */

#include <logicalaccess/plugins/readers/pcsc/commands/mifarespringcardcommands.hpp>

#include <iostream>
#include <iomanip>
#include <sstream>
#include <cassert>

#include <logicalaccess/plugins/readers/pcsc/pcscreaderprovider.hpp>
#include <logicalaccess/plugins/cards/mifare/mifarechip.hpp>

namespace logicalaccess
{
MifareSpringCardCommands::MifareSpringCardCommands()
    : MifarePCSCCommands(CMD_MIFARESPRINGCARD)
{
}

MifareSpringCardCommands::MifareSpringCardCommands(std::string ct)
    : MifarePCSCCommands(ct)
{
}

MifareSpringCardCommands::~MifareSpringCardCommands()
{
}

bool MifareSpringCardCommands::loadKey(unsigned char keyno, MifareKeyType keytype,
                                       std::shared_ptr<MifareKey> key, bool vol)
{
    bool r = false;

    ByteVector vector_key((unsigned char *)key->getData(),
                          (unsigned char *)key->getData() + key->getLength());

    unsigned char keyindex = 0x00;
    if (keytype == KT_KEY_B)
    {
        keyindex = 0x10 + keyno;
    }

    ByteVector result = getPCSCReaderCardAdapter()->sendAPDUCommand(
        0xFF, 0x82, 0x00, keyindex, static_cast<unsigned char>(vector_key.size()),
        vector_key);

    if (!vol && (result[result.size() - 2] == 0x63) &&
        (result[result.size() - 1] == 0x86))
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

void MifareSpringCardCommands::authenticate(unsigned char blockno, unsigned char keyno,
                                            MifareKeyType keytype)
{
    ByteVector command;

    unsigned char keyindex = 0x00 + keyno;
    if (keytype == KT_KEY_B)
    {
        keyindex = 0x10 + keyno;
    }

    command.push_back(0x01);
    command.push_back(0x00);
    command.push_back(blockno);
    command.push_back(0x00);
    command.push_back(keyindex);

    getPCSCReaderCardAdapter()->sendAPDUCommand(
        0xFF, 0x86, 0x00, 0x00, static_cast<unsigned char>(command.size()), command);
}

void MifareSpringCardCommands::restore(unsigned char blockno)
{
    ByteVector buf;
    buf.push_back(0x00);
    buf.push_back(0x00);
    buf.push_back(0x00);
    buf.push_back(0x00);

    getPCSCReaderCardAdapter()->sendAPDUCommand(
        0xFF, 0xF5, 0xC2, blockno, static_cast<unsigned char>(buf.size()), buf);
}

void MifareSpringCardCommands::increment(unsigned char blockno, uint32_t value)
{
    ByteVector buf;

    buf.push_back(static_cast<unsigned char>(value & 0xff));
    buf.push_back(static_cast<unsigned char>((value >> 8) & 0xff));
    buf.push_back(static_cast<unsigned char>((value >> 16) & 0xff));
    buf.push_back(static_cast<unsigned char>((value >> 24) & 0xff));
    getPCSCReaderCardAdapter()->sendAPDUCommand(
        0xFF, 0xF5, 0xC1, blockno, static_cast<unsigned char>(buf.size()), buf);
}

void MifareSpringCardCommands::decrement(unsigned char blockno, uint32_t value)
{
    ByteVector buf;

    buf.push_back(static_cast<unsigned char>(value & 0xff));
    buf.push_back(static_cast<unsigned char>((value >> 8) & 0xff));
    buf.push_back(static_cast<unsigned char>((value >> 16) & 0xff));
    buf.push_back(static_cast<unsigned char>((value >> 24) & 0xff));
    getPCSCReaderCardAdapter()->sendAPDUCommand(
        0xFF, 0xF5, 0xC0, blockno, static_cast<unsigned char>(buf.size()), buf);
}
}