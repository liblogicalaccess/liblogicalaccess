/**
 * \file mifaresmartidcommands.cpp
 * \author Julien K. <julien-dev@islog.com>, Maxime C. <maxime-dev@islog.com>
 * \brief Mifare SmartID commands.
 */

#include <string.h>
#include "mifaresmartidcommands.hpp"
#include "../smartidreaderprovider.hpp"
#include "mifare/mifarechip.hpp"
#include "mifare/mifarelocation.hpp"
#include "logicalaccess/cards/computermemorykeystorage.hpp"
#include "logicalaccess/cards/readermemorykeystorage.hpp"
#include "logicalaccess/cards/samkeystorage.hpp"
#include "logicalaccess/myexception.hpp"

namespace logicalaccess
{
MifareSmartIDCommands::MifareSmartIDCommands()
    : MifareCommands(CMD_MIFARESMARTID)
{
}

MifareSmartIDCommands::MifareSmartIDCommands(std::string ct)
    : MifareCommands(ct)
{
}

MifareSmartIDCommands::~MifareSmartIDCommands()
{
}

bool MifareSmartIDCommands::loadKey(unsigned char keyno, MifareKeyType keytype,
                                    std::shared_ptr<MifareKey> key, bool /*vol*/)
{
    ByteVector data;
    data.resize(14, 0x00);

    data[0] = ((keytype == KT_KEY_A) ? 0 : 4) | 3;
    data[1] = keyno;
    memcpy(&(data[8]), key->getData(), key->getLength());

    getMifareSmartIDReaderCardAdapter()->sendCommand(0x4C, data);

    return true;
}

void MifareSmartIDCommands::loadKey(std::shared_ptr<Location> location,
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
        // Don't load the key when reader memory

        // std::shared_ptr<ReaderMemoryKeyStorage> rmKs =
        // std::dynamic_pointer_cast<ReaderMemoryKeyStorage>(key_storage);
        // loadKey(rmKs->getKeySlot(), keytype, key->getData(), key->getLength(),
        // rmKs->getVolatile());
    }
    else
    {
        THROW_EXCEPTION_WITH_LOG(
            LibLogicalAccessException,
            "The key storage type is not supported for this card/reader.");
    }
}

void MifareSmartIDCommands::authenticate(unsigned char blockno, unsigned char keyno,
                                         MifareKeyType keytype)
{
    ByteVector data;
    data.resize(3, 0x00);

    data[0] = ((keytype == KT_KEY_A) ? 0 : 4) | 3;
    data[1] = keyno;
    data[2] = blockno;

    getMifareSmartIDReaderCardAdapter()->sendCommand(0x56, data);
}

void MifareSmartIDCommands::authenticate(unsigned char blockno,
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

ByteVector MifareSmartIDCommands::readBinary(unsigned char blockno, size_t len)
{
    if (len >= 256)
    {
        THROW_EXCEPTION_WITH_LOG(std::invalid_argument, "Bad len parameter.");
    }

    ByteVector data;
    data.push_back(blockno);

    ByteVector sbuf;
    sbuf = getMifareSmartIDReaderCardAdapter()->sendCommand(0x46, data);

    EXCEPTION_ASSERT_WITH_LOG(sbuf.size() == 16, LibLogicalAccessException,
                              "The read value should always be 16 bytes long");

    return sbuf;
}

void MifareSmartIDCommands::updateBinary(unsigned char blockno, const ByteVector &buf)
{
    if (buf.size() >= 256)
    {
        THROW_EXCEPTION_WITH_LOG(std::invalid_argument, "Bad len parameter.");
    }

    if (blockno != 0)
    {
        ByteVector ldata;

        ldata.push_back(blockno);
        ldata.insert(ldata.end(), buf.begin(), buf.end());

        getMifareSmartIDReaderCardAdapter()->sendCommand(0x47, ldata);
    }
}

void MifareSmartIDCommands::increment(unsigned char blockno, uint32_t value)
{
    increment_raw(blockno, value);
    transfer(blockno);
}

void MifareSmartIDCommands::decrement(unsigned char blockno, uint32_t value)
{
    decrement_raw(blockno, value);
    transfer(blockno);
}

void MifareSmartIDCommands::transfer(unsigned char blockno)
{
    ByteVector command;
    command.push_back(blockno);

    getMifareSmartIDReaderCardAdapter()->sendCommand(0x4B, command);
}

void MifareSmartIDCommands::restore(unsigned char blockno)
{
    ByteVector command;
    command.push_back(blockno);

    getMifareSmartIDReaderCardAdapter()->sendCommand(0x4A, command);
}

void MifareSmartIDCommands::increment_raw(uint8_t blockno, uint32_t value) const
{
    ByteVector command;
    command.push_back(blockno);
    command.push_back(static_cast<unsigned char>(value & 0xff));
    command.push_back(static_cast<unsigned char>((value >> 8) & 0xff));
    command.push_back(static_cast<unsigned char>((value >> 16) & 0xff));
    command.push_back(static_cast<unsigned char>((value >> 24) & 0xff));

    getMifareSmartIDReaderCardAdapter()->sendCommand(0x48, command);
}

void MifareSmartIDCommands::decrement_raw(uint8_t blockno, uint32_t value) const
{
    ByteVector command;
    command.push_back(blockno);
    command.push_back(static_cast<unsigned char>(value & 0xff));
    command.push_back(static_cast<unsigned char>((value >> 8) & 0xff));
    command.push_back(static_cast<unsigned char>((value >> 16) & 0xff));
    command.push_back(static_cast<unsigned char>((value >> 24) & 0xff));

    getMifareSmartIDReaderCardAdapter()->sendCommand(0x49, command);
}
}