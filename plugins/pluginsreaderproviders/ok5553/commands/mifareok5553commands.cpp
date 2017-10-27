/**
 * \file mifareok5553commands.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Mifaire commands for OK5553 readers.
 */

#include "mifareok5553commands.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>

#include "mifare/mifarechip.hpp"
#include "mifare/mifarelocation.hpp"
#include "logicalaccess/cards/computermemorykeystorage.hpp"
#include "logicalaccess/cards/readermemorykeystorage.hpp"
#include "logicalaccess/cards/samkeystorage.hpp"
#include "logicalaccess/myexception.hpp"

namespace logicalaccess
{
MifareOK5553Commands::MifareOK5553Commands()
    : MifareCommands(CMD_MIFAREOK5553)
{
}

MifareOK5553Commands::MifareOK5553Commands(std::string ct)
    : MifareCommands(ct)
{
}

MifareOK5553Commands::~MifareOK5553Commands()
{
}

ByteVector MifareOK5553Commands::readBinary(unsigned char blockno, size_t /*len*/)
{
    char tmp[3];
    ByteVector command;
    command.push_back(static_cast<unsigned char>('r'));
    command.push_back(static_cast<unsigned char>('b'));
    sprintf(tmp, "%.2X", blockno);
    command.push_back(static_cast<unsigned char>(tmp[0]));
    command.push_back(static_cast<unsigned char>(tmp[1]));
    ByteVector answer = getOK5553ReaderCardAdapter()->sendCommand(command);
    // convert ascii in hexa
    return OK5553ReaderUnit::asciiToHex(answer);
}

void MifareOK5553Commands::updateBinary(unsigned char blockno, const ByteVector &buf)
{
    char tmp[3];
    ByteVector command;
    command.push_back(static_cast<unsigned char>('w'));
    command.push_back(static_cast<unsigned char>('b'));
    sprintf(tmp, "%.2X", blockno);
    command.push_back(static_cast<unsigned char>(tmp[0]));
    command.push_back(static_cast<unsigned char>(tmp[1]));
    for (size_t i = 0; i < buf.size(); i++)
    {
        sprintf(tmp, "%.2X", buf[i]);
        command.push_back(static_cast<unsigned char>(tmp[0]));
        command.push_back(static_cast<unsigned char>(tmp[1]));
    }
    getOK5553ReaderCardAdapter()->sendCommand(command);
}

bool MifareOK5553Commands::loadKey(unsigned char keyno, MifareKeyType /*keytype*/,
                                   std::shared_ptr<MifareKey> key, bool vol)
{
    bool r = true;
    if (!vol)
    {
        char buf[3];
        ByteVector command;
        command.push_back(static_cast<unsigned char>('w'));
        command.push_back(static_cast<unsigned char>('m'));
        sprintf(buf, "%.2X", keyno);
        command.push_back(static_cast<unsigned char>(buf[0]));
        command.push_back(static_cast<unsigned char>(buf[1]));
        for (size_t i = 0; i < key->getLength(); i++)
        {
            sprintf(buf, "%.2X", key->getData()[i]);
            command.push_back(static_cast<unsigned char>(buf[0]));
            command.push_back(static_cast<unsigned char>(buf[1]));
        }
        ByteVector answer = getOK5553ReaderCardAdapter()->sendCommand(command);
        if (answer.size() < 2)
            r = false;
    }
    return r;
}

void MifareOK5553Commands::loadKey(std::shared_ptr<Location> location,
                                   MifareKeyType keytype, std::shared_ptr<MifareKey> key)
{
    EXCEPTION_ASSERT_WITH_LOG(location, std::invalid_argument,
                              "location cannot be null.");
    EXCEPTION_ASSERT_WITH_LOG(key, std::invalid_argument, "key cannot be null.");

    std::shared_ptr<MifareLocation> mLocation =
        std::dynamic_pointer_cast<MifareLocation>(location);
    std::shared_ptr<MifareKey> mKey = std::dynamic_pointer_cast<MifareKey>(key);

    EXCEPTION_ASSERT_WITH_LOG(mLocation, std::invalid_argument,
                              "location must be a MifareLocation.");
    EXCEPTION_ASSERT_WITH_LOG(mKey, std::invalid_argument, "key must be a MifareKey.");

    std::shared_ptr<KeyStorage> key_storage = key->getKeyStorage();

    if (std::dynamic_pointer_cast<ComputerMemoryKeyStorage>(key_storage))
    {
        loadKey(0, keytype, key);
    }
    else if (std::dynamic_pointer_cast<ReaderMemoryKeyStorage>(key_storage))
    {
        // Don't load the key when reader memory, except if specified
        if (!key->isEmpty())
        {
            std::shared_ptr<ReaderMemoryKeyStorage> rmKs =
                std::dynamic_pointer_cast<ReaderMemoryKeyStorage>(key_storage);
            loadKey(rmKs->getKeySlot(), keytype, key, rmKs->getVolatile());
        }
    }
    else
    {
        THROW_EXCEPTION_WITH_LOG(
            LibLogicalAccessException,
            "The key storage type is not supported for this card/reader.");
    }
}

void MifareOK5553Commands::authenticate(unsigned char blockno, unsigned char keyno,
                                        MifareKeyType keytype)
{
    ByteVector command;
    char buf[3];
    command.push_back(static_cast<unsigned char>('l'));
    unsigned char sector = blockno / 4;
    sprintf(buf, "%.2X", sector);
    command.push_back(static_cast<unsigned char>(buf[0]));
    command.push_back(static_cast<unsigned char>(buf[1]));
    sprintf(buf, "%.2X", keyno);
    if (keytype == KT_KEY_A)
        buf[0] += 1;
    else
        buf[0] += 3;
    command.push_back(static_cast<unsigned char>(buf[0]));
    command.push_back(static_cast<unsigned char>(buf[1]));
    ByteVector answer = getOK5553ReaderCardAdapter()->sendCommand(command);
    if (answer.size() > 0)
    {
        if (answer[0] != 'L')
        {
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Authentication error.");
        }
    }
}

// in progress
void MifareOK5553Commands::authenticate(unsigned char blockno,
                                        std::shared_ptr<KeyStorage> key_storage,
                                        MifareKeyType keytype)
{
    if (std::dynamic_pointer_cast<ComputerMemoryKeyStorage>(key_storage))
    {
        authenticate(blockno, 0, keytype);
    }
    else if (std::dynamic_pointer_cast<ReaderMemoryKeyStorage>(key_storage))
    {
        std::shared_ptr<ReaderMemoryKeyStorage> rmKs =
            std::dynamic_pointer_cast<ReaderMemoryKeyStorage>(key_storage);
        authenticate(blockno, rmKs->getKeySlot(), keytype);
    }
    else
    {
        THROW_EXCEPTION_WITH_LOG(
            LibLogicalAccessException,
            "The key storage type is not supported for this card/reader.");
    }
}

void MifareOK5553Commands::increment(unsigned char blockno, unsigned int value)
{
    ByteVector command;
    command.push_back(static_cast<unsigned char>('+'));
    command.push_back(blockno);
    command.push_back(static_cast<unsigned char>(value & 0xff));
    command.push_back(static_cast<unsigned char>((value >> 8) & 0xff));
    command.push_back(static_cast<unsigned char>((value >> 16) & 0xff));
    command.push_back(static_cast<unsigned char>((value >> 24) & 0xff));

    ByteVector answer = getOK5553ReaderCardAdapter()->sendCommand(command);

    if (answer.size() > 0)
    {
        if (answer[0] == 'l')
        {
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Value block failure.");
        }
        if (answer[0] == 'F')
        {
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Increment failure.");
        }
    }
}

void MifareOK5553Commands::decrement(unsigned char blockno, unsigned int value)
{
    ByteVector command;
    command.push_back(static_cast<unsigned char>('-'));
    command.push_back(blockno);
    command.push_back(static_cast<unsigned char>(value & 0xff));
    command.push_back(static_cast<unsigned char>((value >> 8) & 0xff));
    command.push_back(static_cast<unsigned char>((value >> 16) & 0xff));
    command.push_back(static_cast<unsigned char>((value >> 24) & 0xff));

    ByteVector answer = getOK5553ReaderCardAdapter()->sendCommand(command);

    if (answer.size() > 0)
    {
        if (answer[0] == 'l')
        {
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Value block failure.");
        }
        if (answer[0] == 'F')
        {
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Decrement failure.");
        }
    }
}
}