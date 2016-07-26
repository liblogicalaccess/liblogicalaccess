/**
 * \file mifarepcsccommands.cpp
 * \author Julien K. <julien-dev@islog.com>, Maxime C. <maxime-dev@islog.com>
 * \brief Mifare PC/SC commands.
 */

#include "../commands/mifarepcsccommands.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>

#include "../pcscreaderprovider.hpp"
#include "mifarechip.hpp"
#include "mifarelocation.hpp"
#include "logicalaccess/cards/computermemorykeystorage.hpp"
#include "logicalaccess/cards/readermemorykeystorage.hpp"
#include "logicalaccess/cards/samkeystorage.hpp"
#include "logicalaccess/myexception.hpp"

namespace logicalaccess
{
    MifarePCSCCommands::MifarePCSCCommands()
        : MifareCommands()
    {
    }

    MifarePCSCCommands::~MifarePCSCCommands()
    {
    }

    bool MifarePCSCCommands::loadKey(unsigned char keyno, MifareKeyType keytype, std::shared_ptr<MifareKey> key, bool vol)
    {
        bool r = false;
        std::vector<unsigned char> result,
			vector_key((unsigned char *)key->getData(), (unsigned char *)key->getData() + key->getLength());

        try
        {
            result = getPCSCReaderCardAdapter()->sendAPDUCommand(
                0xFF, 0x82, (vol ? 0x00 : 0x20), static_cast<char>(keyno),
                static_cast<unsigned char>(vector_key.size()), vector_key);
        }
        catch (CardException e)
        {
            if (!vol && (e.error_code() == CardException::WRONG_P1_P2 ||
                         e.error_code() == CardException::UNKOWN_ERROR))
            {
                // With the Sony RC-S380, non-volatile memory doesn't work,
                // so we try again. Same with ACR1222L.
                // Apparently the issue is also hit with ACS122U.
                return loadKey(keyno, keytype, key, true);
            }
            else
                throw;
        }
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

	void MifarePCSCCommands::loadKey(std::shared_ptr<Location> location, MifareKeyType keytype, std::shared_ptr<MifareKey> key)
    {
        EXCEPTION_ASSERT_WITH_LOG(location, std::invalid_argument, "location cannot be null.");
        EXCEPTION_ASSERT_WITH_LOG(key, std::invalid_argument, "key cannot be null.");

        std::shared_ptr<MifareLocation> mLocation = std::dynamic_pointer_cast<MifareLocation>(location);
        std::shared_ptr<MifareKey> mKey = std::dynamic_pointer_cast<MifareKey>(key);

        EXCEPTION_ASSERT_WITH_LOG(mLocation, std::invalid_argument, "location must be a MifareLocation.");
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
                std::shared_ptr<ReaderMemoryKeyStorage> rmKs = std::dynamic_pointer_cast<ReaderMemoryKeyStorage>(key_storage);
                loadKey(rmKs->getKeySlot(), keytype, key, rmKs->getVolatile());
            }
        }
        else
        {
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "The key storage type is not supported for this card/reader.");
        }
    }

    void MifarePCSCCommands::authenticate(unsigned char blockno, unsigned char keyno, MifareKeyType keytype)
    {
        TRACE(blockno, keyno, keytype);
        std::vector<unsigned char> command;

        command.push_back(0x01);
        command.push_back(0x00);
        command.push_back(blockno);
        command.push_back(static_cast<unsigned char>(keytype));
        command.push_back(keyno);

        getPCSCReaderCardAdapter()->sendAPDUCommand(0xFF, 0x86, 0x00, 0x00, static_cast<unsigned char>(command.size()), command);
    }

    void MifarePCSCCommands::authenticate(unsigned char blockno, std::shared_ptr<KeyStorage> key_storage, MifareKeyType keytype)
    {
        TRACE(blockno, key_storage, keytype);
        if (std::dynamic_pointer_cast<ComputerMemoryKeyStorage>(key_storage))
        {
            authenticate(blockno, 0, keytype);
        }
        else if (std::dynamic_pointer_cast<ReaderMemoryKeyStorage>(key_storage))
        {
            std::shared_ptr<ReaderMemoryKeyStorage> rmKs = std::dynamic_pointer_cast<ReaderMemoryKeyStorage>(key_storage);
            authenticate(blockno, rmKs->getKeySlot(), keytype);
        }
        else
        {
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "The key storage type is not supported for this card/reader.");
        }
    }

    std::vector<unsigned char> MifarePCSCCommands::readBinary(unsigned char blockno, size_t len)
    {
        TRACE(blockno, len);
        if (len >= 256)
        {
            THROW_EXCEPTION_WITH_LOG(std::invalid_argument, "Bad len parameter.");
        }

        std::vector<unsigned char> result;

        result = getPCSCReaderCardAdapter()->sendAPDUCommand(0xFF, 0xB0, 0x00, blockno, static_cast<unsigned char>(len));
        return std::vector<unsigned char>(result.begin(), result.end() - 2);
    }

    void MifarePCSCCommands::updateBinary(unsigned char blockno, const std::vector<unsigned char>& buf)
    {
        TRACE(blockno, buf);
        if (buf.size() >= 256)
        {
            THROW_EXCEPTION_WITH_LOG(std::invalid_argument, "Bad buf size parameter.");
        }

        getPCSCReaderCardAdapter()->sendAPDUCommand(0xFF, 0xD6, 0x00, blockno, static_cast<unsigned char>(buf.size()), buf);
    }

    void MifarePCSCCommands::increment(uint8_t blockno, uint32_t value)
    {
        std::vector<uint8_t> buf;
        std::vector<uint8_t> dest           = {0x80, 0x01, blockno};
        std::vector<uint8_t> value_change   = {0x81, 0x04, 0x00, 0x00, 0x00, 0x00};

        memcpy(&value_change[2], &value, 4);

        buf = {0xA0, static_cast<uint8_t>(dest.size() + value_change.size())};
        buf.insert(buf.end(), dest.begin(), dest.end());
        buf.insert(buf.end(), value_change.begin(), value_change.end());

        getPCSCReaderCardAdapter()->sendAPDUCommand(0xFF, 0xC2, 0x00,
                                                    0x03, static_cast<unsigned char>(buf.size()),
                                                    buf);
    }

    void MifarePCSCCommands::decrement(uint8_t blockno, uint32_t value)
    {
        std::vector<uint8_t> buf;
        std::vector<uint8_t> dest           = {0x80, 0x01, blockno};
        std::vector<uint8_t> value_change   = {0x81, 0x04, 0x00, 0x00, 0x00, 0x00};

        memcpy(&value_change[2], &value, 4);

        buf = {0xA1, static_cast<uint8_t>(dest.size() + value_change.size())};
        buf.insert(buf.end(), dest.begin(), dest.end());
        buf.insert(buf.end(), value_change.begin(), value_change.end());

        getPCSCReaderCardAdapter()->sendAPDUCommand(0xFF, 0xC2, 0x00,
                                                    0x03, static_cast<unsigned char>(buf.size()),
                                                    buf);
    }

}
