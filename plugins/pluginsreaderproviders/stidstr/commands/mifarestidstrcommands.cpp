/**
 * \file mifarestidstrcommands.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Mifare STidSTR commands.
 */

#include "mifarestidstrcommands.hpp"
#include "../stidstrreaderprovider.hpp"
#include "mifare/mifarechip.hpp"
#include "mifare/mifarelocation.hpp"
#include "logicalaccess/cards/computermemorykeystorage.hpp"
#include "logicalaccess/cards/readermemorykeystorage.hpp"
#include "logicalaccess/cards/samkeystorage.hpp"
#include "logicalaccess/myexception.hpp"

namespace logicalaccess
{
    MifareSTidSTRCommands::MifareSTidSTRCommands()
        : MifareCommands(CMD_MIFARESTIDSTR)
    {
        d_useSKB = false;
        d_skbIndex = 0;
        d_lastKeyType = KT_KEY_A;
    }

	MifareSTidSTRCommands::MifareSTidSTRCommands(std::string ct)
		: MifareCommands(ct)
	{
		d_useSKB = false;
		d_skbIndex = 0;
		d_lastKeyType = KT_KEY_A;
	}

    MifareSTidSTRCommands::~MifareSTidSTRCommands()
    {
    }

    std::vector<unsigned char> MifareSTidSTRCommands::scanMifare()
    {
        LOG(LogLevel::INFOS) << "Scanning mifare card...";
        std::vector<unsigned char> uid;
        std::vector<unsigned char> r = getSTidSTRReaderCardAdapter()->sendCommand(0x00A0, std::vector<unsigned char>());

        bool hasCard = (r[0] == 0x01);
        if (hasCard)
        {
            LOG(LogLevel::INFOS) << "Card detected !";
            unsigned char uidLength = r[1];
            uid = std::vector<unsigned char>(r.begin() + 2, r.begin() + 2 + uidLength);

            LOG(LogLevel::INFOS) << "Card uid " << BufferHelper::getHex(uid) << "-{" << BufferHelper::getStdString(uid) << "}";
        }
        else
        {
            LOG(LogLevel::INFOS) << "No card detected !";
        }

        return uid;
    }

    void MifareSTidSTRCommands::releaseRFIDField()
    {
        LOG(LogLevel::INFOS) << "Releasing RFID field...";
        getSTidSTRReaderCardAdapter()->sendCommand(0x00A1, std::vector<unsigned char>());
    }

    bool MifareSTidSTRCommands::loadKey(unsigned char keyno, MifareKeyType keytype, std::shared_ptr<MifareKey> key, bool vol)
    {
        LOG(LogLevel::INFOS) << "Loading key... key number {0x" << std::hex << keyno << std::dec << "(" << keyno << ")} key type {0x" << std::hex << keytype << std::dec << "(" << keytype << ")} key len {" << key->getLength() << "} volatile ? {" << vol << "}";
        std::vector<unsigned char> command;
        command.push_back(static_cast<unsigned char>(keytype));
        command.push_back(vol ? 0x00 : 0x01);
        command.push_back(keyno);
		command.insert(command.end(), key->getData(), key->getData() + key->getLength());
        command.push_back(0x00);	// Diversify ?

        getSTidSTRReaderCardAdapter()->sendCommand(0x00D0, command);

        return true;
    }

	void MifareSTidSTRCommands::loadKey(std::shared_ptr<Location> location, MifareKeyType keytype, std::shared_ptr<MifareKey> key)
    {
        LOG(LogLevel::INFOS) << "Loading key... location {" << location->serialize() << "} key type {0x" << std::hex << keytype << std::dec << "(" << keytype << ")}";

        EXCEPTION_ASSERT_WITH_LOG(location, std::invalid_argument, "location cannot be null.");
        EXCEPTION_ASSERT_WITH_LOG(key, std::invalid_argument, "key cannot be null.");

        std::shared_ptr<MifareLocation> mLocation = std::dynamic_pointer_cast<MifareLocation>(location);
        std::shared_ptr<MifareKey> mKey = std::dynamic_pointer_cast<MifareKey>(key);

        EXCEPTION_ASSERT_WITH_LOG(mLocation, std::invalid_argument, "location must be a MifareLocation.");
        EXCEPTION_ASSERT_WITH_LOG(mKey, std::invalid_argument, "key must be a MifareKey.");

        std::shared_ptr<KeyStorage> key_storage = key->getKeyStorage();

        if (std::dynamic_pointer_cast<ComputerMemoryKeyStorage>(key_storage))
        {
            LOG(LogLevel::INFOS) << "Using computer memory key storage !";
            loadKey(static_cast<unsigned char>(mLocation->sector), keytype, key, true);
        }
        else if (std::dynamic_pointer_cast<ReaderMemoryKeyStorage>(key_storage))
        {
            LOG(LogLevel::INFOS) << "Using reader memory key storage !";
            // Don't load the key when reader memory, except if specified
            if (!key->isEmpty())
            {
                THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "It's not possible to change a key at specific index through host/reader connection. Please use SKB configuration card instead.");
            }
        }
        else
        {
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "The key storage type is not supported for this card/reader.");
        }
    }

    void MifareSTidSTRCommands::authenticate(unsigned char /*blockno*/, unsigned char /*keyno*/, MifareKeyType /*keytype*/)
    {
        // STid STR doesn't separate authentication and read/write operation.
        LOG(LogLevel::WARNINGS) << "STid STR doesn't separate authentication and read/write operation.";
    }

    void MifareSTidSTRCommands::authenticate(unsigned char /*blockno*/, std::shared_ptr<KeyStorage> key_storage, MifareKeyType keytype)
    {
        LOG(LogLevel::INFOS) << "Setting the authenticate type... key storage {" << key_storage->serialize() << "} key type {0x" << std::hex << keytype << std::dec << "(" << keytype << ")}";
        if (std::dynamic_pointer_cast<ComputerMemoryKeyStorage>(key_storage))
        {
            LOG(LogLevel::INFOS) << "Setting computer memory key storage !";
            d_useSKB = false;
            d_skbIndex = 0;
        }
        else if (std::dynamic_pointer_cast<ReaderMemoryKeyStorage>(key_storage))
        {
            d_useSKB = true;
            std::shared_ptr<ReaderMemoryKeyStorage> rmks = std::dynamic_pointer_cast<ReaderMemoryKeyStorage>(key_storage);
            d_skbIndex = rmks->getKeySlot();
            LOG(LogLevel::INFOS) << "Setting reader memory key storage ! SKB index {" << d_skbIndex << "}";
        }
        else
        {
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "The key storage type is not supported for this card/reader.");
        }
        d_lastKeyType = keytype;
    }

    std::vector<unsigned char> MifareSTidSTRCommands::readBinary(unsigned char blockno, size_t len)
    {
        LOG(LogLevel::INFOS) << "Read binary block {0x" << std::hex << blockno << std::dec << "(" << blockno << ")} len {" << len << "}";
        if (len >= 256)
        {
            THROW_EXCEPTION_WITH_LOG(std::invalid_argument, "Bad length parameter.");
        }

        if (d_useSKB)
        {
            LOG(LogLevel::INFOS) << "Need to use reader memory key storage (SKB) !";
            return readBinaryIndex(d_skbIndex, blockno, len);
        }

        LOG(LogLevel::INFOS) << " => Rescanning card to avoid bad authentication";
        scanMifare();
        LOG(LogLevel::INFOS) << "Scan done ! Continue to read binary block.";

        std::vector<unsigned char> command;
        command.push_back(static_cast<unsigned char>(d_lastKeyType));
        command.push_back(blockno);

        std::vector<unsigned char> sbuf = getSTidSTRReaderCardAdapter()->sendCommand(0x00B2, command);

        LOG(LogLevel::INFOS) << "Read binary buffer returned " << BufferHelper::getHex(sbuf) << " len {" << sbuf.size() << "}";
        EXCEPTION_ASSERT_WITH_LOG(sbuf.size() == 16, LibLogicalAccessException, "The read value should always be 16 bytes long");

        return sbuf;
    }

    std::vector<unsigned char> MifareSTidSTRCommands::readBinaryIndex(unsigned char keyindex, unsigned char blockno, size_t /*len*/)
    {
        LOG(LogLevel::INFOS) << "Read binary index key index {0x" << std::hex << keyindex << std::dec << "(" << keyindex << ")} block {0x" << std::hex << blockno << std::dec << "(" << blockno << ")}";
        LOG(LogLevel::INFOS) << " => Rescanning card to avoid bad authentication";
        scanMifare();
        LOG(LogLevel::INFOS) << "Scan done ! Continue to read binary index.";

        std::vector<unsigned char> command;
        command.push_back(static_cast<unsigned char>(d_lastKeyType));
        command.push_back(keyindex);
        command.push_back(blockno);

        std::vector<unsigned char> sbuf = getSTidSTRReaderCardAdapter()->sendCommand(0x00B1, command);

        EXCEPTION_ASSERT_WITH_LOG(sbuf.size() == 16, LibLogicalAccessException, "The read value should always be 16 bytes long");

        return sbuf;
    }

    void MifareSTidSTRCommands::updateBinary(unsigned char blockno, const std::vector<unsigned char>& buf)
    {
        LOG(LogLevel::INFOS) << "Update binary block {0x" << std::hex << blockno << std::dec << "(" << blockno << ")}";

        if (d_useSKB)
        {
            LOG(LogLevel::INFOS) << "Need to use reader memory key storage (SKB) !";
            return updateBinaryIndex(d_skbIndex, blockno, buf);
        }

        LOG(LogLevel::INFOS) << " => Rescanning card to avoid bad authentication";
        scanMifare();
        LOG(LogLevel::INFOS) << "Scan done ! Continue to update binary block.";

        if (blockno != 0)
        {
            std::vector<unsigned char> command;
            command.push_back(static_cast<unsigned char>(d_lastKeyType));
            command.push_back(blockno);
            command.insert(command.end(), buf.begin(), buf.end());

            getSTidSTRReaderCardAdapter()->sendCommand(0x00D2, command);
        }
    }

    void MifareSTidSTRCommands::updateBinaryIndex(unsigned char keyindex, unsigned char blockno, const std::vector<unsigned char>& buf)
    {
        LOG(LogLevel::INFOS) << "Update binary index key index {0x" << std::hex << keyindex << std::dec << "(" << keyindex << ")} block {0x" << std::hex << blockno << std::dec << "(" << blockno << ")}";

        LOG(LogLevel::INFOS) << " => Rescanning card to avoid bad authentication";
        scanMifare();
        LOG(LogLevel::INFOS) << "Scan done ! Continue to update binary index.";

        if (blockno != 0)
        {
            std::vector<unsigned char> command;
            command.push_back(static_cast<unsigned char>(d_lastKeyType));
            command.push_back(keyindex);
            command.push_back(blockno);
            command.insert(command.end(), buf.begin(), buf.end());

            getSTidSTRReaderCardAdapter()->sendCommand(0x00D3, command);
        }
    }

	void MifareSTidSTRCommands::increment(unsigned char /*blockno*/, uint32_t /*value*/)
	{
		THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Not implemented.");
	}

	void MifareSTidSTRCommands::decrement(unsigned char /*blockno*/,  uint32_t /*value*/)
	{
		THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Not implemented.");
	}
}