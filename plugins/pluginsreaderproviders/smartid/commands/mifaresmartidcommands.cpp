/**
 * \file mifaresmartidcommands.cpp
 * \author Julien K. <julien-dev@islog.com>, Maxime C. <maxime-dev@islog.com>
 * \brief Mifare SmartID commands.
 */

#include "mifaresmartidcommands.hpp"
#include "../smartidreaderprovider.hpp"
#include "mifarechip.hpp"
#include "logicalaccess/cards/computermemorykeystorage.hpp"
#include "logicalaccess/cards/readermemorykeystorage.hpp"
#include "logicalaccess/cards/samkeystorage.hpp"

namespace logicalaccess
{
    MifareSmartIDCommands::MifareSmartIDCommands()
        : MifareCommands()
    {
    }

    MifareSmartIDCommands::~MifareSmartIDCommands()
    {
    }

    bool MifareSmartIDCommands::loadKey(unsigned char keyno, MifareKeyType keytype, const void* key, size_t keylen, bool /*vol*/)
    {
        std::vector<unsigned char> data;
        data.resize(14, 0x00);

        data[0] = ((keytype == KT_KEY_A) ? 0 : 4) | 3;
        data[1] = keyno;
        memcpy(&(data[8]), key, keylen);

        getMifareSmartIDReaderCardAdapter()->sendCommand(0x4C, data);

        return true;
    }

    void MifareSmartIDCommands::loadKey(std::shared_ptr<Location> location, std::shared_ptr<Key> key, MifareKeyType keytype)
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
            loadKey(0, keytype, key->getData(), key->getLength());
        }
        else if (std::dynamic_pointer_cast<ReaderMemoryKeyStorage>(key_storage))
        {
            // Don't load the key when reader memory

            //std::shared_ptr<ReaderMemoryKeyStorage> rmKs = std::dynamic_pointer_cast<ReaderMemoryKeyStorage>(key_storage);
            //loadKey(rmKs->getKeySlot(), keytype, key->getData(), key->getLength(), rmKs->getVolatile());
        }
        else
        {
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "The key storage type is not supported for this card/reader.");
        }
    }

    void MifareSmartIDCommands::authenticate(unsigned char blockno, unsigned char keyno, MifareKeyType keytype)
    {
        std::vector<unsigned char> data;
        data.resize(3, 0x00);

        data[0] = ((keytype == KT_KEY_A) ? 0 : 4) | 3;
        data[1] = keyno;
        data[2] = blockno;

        getMifareSmartIDReaderCardAdapter()->sendCommand(0x56, data);
    }

    void MifareSmartIDCommands::authenticate(unsigned char blockno, std::shared_ptr<KeyStorage> key_storage, MifareKeyType keytype)
    {
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

    size_t MifareSmartIDCommands::readBinary(unsigned char blockno, size_t len, void* buf, size_t buflen)
    {
        if ((len >= 256) || (len > buflen) || (!buf))
        {
            THROW_EXCEPTION_WITH_LOG(std::invalid_argument, "Bad buffer parameter.");
        }

        std::vector<unsigned char> data;
        data.push_back(blockno);

        std::vector<unsigned char> sbuf;
        sbuf = getMifareSmartIDReaderCardAdapter()->sendCommand(0x46, data);

        EXCEPTION_ASSERT_WITH_LOG(sbuf.size() == 16, LibLogicalAccessException, "The read value should always be 16 bytes long");

        memcpy(buf, &sbuf[0], sbuf.size());

        return sbuf.size();
    }

    size_t MifareSmartIDCommands::updateBinary(unsigned char blockno, const void* buf, size_t buflen)
    {
        if ((buflen >= 256) || (!buf))
        {
            THROW_EXCEPTION_WITH_LOG(std::invalid_argument, "Bad buffer parameter.");
        }

        if (blockno != 0)
        {
            std::vector<unsigned char> ldata;

            ldata.push_back(blockno);
            ldata.insert(ldata.end(), static_cast<const unsigned char*>(buf), static_cast<const unsigned char*>(buf)+buflen);

            getMifareSmartIDReaderCardAdapter()->sendCommand(0x47, ldata);
        }

        return buflen;
    }
}