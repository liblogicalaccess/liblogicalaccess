/**
 * \file desfireev1stidstrcommands.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief DESFireEV1 STidSTR commands.
 */

#include "desfireev1stidstrcommands.hpp"
#include "desfire/desfirechip.hpp"
#include <openssl/rand.h>
#include <string.h>
#include "logicalaccess/logs.hpp"
#include "logicalaccess/crypto/aes_cipher.hpp"
#include "logicalaccess/crypto/aes_symmetric_key.hpp"
#include "logicalaccess/crypto/aes_initialization_vector.hpp"
#include "logicalaccess/crypto/des_cipher.hpp"
#include "logicalaccess/crypto/des_symmetric_key.hpp"
#include "logicalaccess/crypto/des_initialization_vector.hpp"
#include "logicalaccess/cards/computermemorykeystorage.hpp"
#include "logicalaccess/cards/readermemorykeystorage.hpp"
#include "logicalaccess/myexception.hpp"

namespace logicalaccess
{
    DESFireEV1STidSTRCommands::DESFireEV1STidSTRCommands()
    {
        d_currentAid = 0;
    }

    DESFireEV1STidSTRCommands::~DESFireEV1STidSTRCommands()
    {
    }

    std::vector<unsigned char> DESFireEV1STidSTRCommands::scanDESFire()
    {
        LOG(LogLevel::INFOS) << "Scanning DESFire card...";
        std::vector<unsigned char> uid;
        std::vector<unsigned char> r = getSTidSTRReaderCardAdapter()->sendCommand(0x0001, std::vector<unsigned char>());

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

    void DESFireEV1STidSTRCommands::releaseRFIDField()
    {
        LOG(LogLevel::INFOS) << "Releasing RFID field...";
        getSTidSTRReaderCardAdapter()->sendCommand(0x0002, std::vector<unsigned char>());
    }

    void DESFireEV1STidSTRCommands::changePPS(STidDESFireBaudrate readerToChipKbps, STidDESFireBaudrate chipToReaderKbps)
    {
        LOG(LogLevel::INFOS) << "Changing PPS (communication speed between the chip and reader)...";
        std::vector<unsigned char> command;
        command.push_back(static_cast<unsigned char>(readerToChipKbps));
        command.push_back(static_cast<unsigned char>(chipToReaderKbps));
        getSTidSTRReaderCardAdapter()->sendCommand(0x0003, command);
    }

    unsigned int DESFireEV1STidSTRCommands::getFreeMem()
    {
        LOG(LogLevel::INFOS) << "Getting free memory of card (number of bytes free available)...";
        unsigned int freemem = 0;
        std::vector<unsigned char> r = getSTidSTRReaderCardAdapter()->sendCommand(0x006E, std::vector<unsigned char>());

        if (r.size() == 3)
        {
            freemem = (r[2] << 16) | (r[1] << 8) | r[0];
        }
        else
        {
            LOG(LogLevel::ERRORS) << "The response buffer size should be 3 !";
        }

        LOG(LogLevel::INFOS) << "Free memory {" << freemem << "}";
        return freemem;
    }

    std::vector<DFName> DESFireEV1STidSTRCommands::getDFNames()
    {
        std::vector<DFName> dfnames;

        LOG(LogLevel::ERRORS) << "Function not available with this reader.";
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Function not available with this reader.");

        return dfnames;
    }

    std::vector<unsigned short> DESFireEV1STidSTRCommands::getISOFileIDs()
    {
        std::vector<unsigned short> fileids;

        LOG(LogLevel::ERRORS) << "Function not available with this reader.";
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Function not available with this reader.");

        return fileids;
    }

    void DESFireEV1STidSTRCommands::createApplication(unsigned int aid, DESFireKeySettings settings, unsigned char maxNbKeys)
    {
        createApplication(aid, settings, maxNbKeys, DF_KEY_DES, FIDS_NO_ISO_FID, 0, std::vector<unsigned char>());
    }

    void DESFireEV1STidSTRCommands::createApplication(unsigned int aid, DESFireKeySettings settings, unsigned char maxNbKeys, DESFireKeyType cryptoMethod, FidSupport /*fidSupported*/, unsigned short isoFID, std::vector<unsigned char> isoDFName)
    {
        LOG(LogLevel::INFOS) << "Creating application aid {0x" << std::hex << aid << std::dec << "(" << aid << ")} settings {0x" << std::hex << settings << std::dec << "(" << settings << ")} max nb Keys {" << maxNbKeys << "} key type {0x" << std::hex << cryptoMethod << std::dec << "(" << cryptoMethod << ")}";

        std::vector<unsigned char> tmpcmd;
        std::vector<unsigned char> command;

        DESFireLocation::convertUIntToAid(aid, tmpcmd);
        command.insert(command.end(), tmpcmd.begin(), tmpcmd.end());
        command.push_back(static_cast<unsigned char>(settings));
        command.push_back(static_cast<unsigned char>(static_cast<unsigned char>(maxNbKeys)& 0x0f));
        command.push_back((cryptoMethod == DF_KEY_AES) ? 0x02 : 0x00);

        if (isoFID != 0x00)
        {
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "isoFID parameter not available with this reader.");
        }
        if (isoDFName.size() > 0)
        {
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "isoDFName parameter not available with this reader.");
        }

        getSTidSTRReaderCardAdapter()->sendCommand(0x00CA, command);

        for (unsigned char i = 0; i < maxNbKeys; ++i)
		{
			getDESFireChip()->getCrypto()->setKey(aid, i, DESFireCrypto::getDefaultKey(cryptoMethod));
        }
    }

    void DESFireEV1STidSTRCommands::deleteApplication(unsigned int aid)
    {
        LOG(LogLevel::INFOS) << "Deleting application aid {0x" << std::hex << aid << std::dec << "(" << aid << ")}...";
        std::vector<unsigned char> command;

        DESFireLocation::convertUIntToAid(aid, command);

        getSTidSTRReaderCardAdapter()->sendCommand(0x00DA, command);
    }

    void DESFireEV1STidSTRCommands::selectApplication(unsigned int aid)
    {
        LOG(LogLevel::INFOS) << "Selecting application aid {0x" << std::hex << aid << std::dec << "(" << aid << ")}";
        std::vector<unsigned char> command;
        DESFireLocation::convertUIntToAid(aid, command);

        getSTidSTRReaderCardAdapter()->sendCommand(0x005A, command);

        d_currentAid = aid;
    }

    void DESFireEV1STidSTRCommands::getKeySettings(DESFireKeySettings& settings, unsigned char& maxNbKeys)
    {
        DESFireKeyType keyType;
        getKeySettings(settings, maxNbKeys, keyType);
    }

    void DESFireEV1STidSTRCommands::getKeySettings(DESFireKeySettings& settings, unsigned char& maxNbKeys, DESFireKeyType& keyType)
    {
        LOG(LogLevel::INFOS) << "Retrieving key settings...";
        std::vector<unsigned char> r = getSTidSTRReaderCardAdapter()->sendCommand(0x0045, std::vector<unsigned char>());

        EXCEPTION_ASSERT_WITH_LOG(r.size() >= 3, LibLogicalAccessException, "The response length should be at least 3-byte long");

        settings = static_cast<DESFireKeySettings>(r[0]);
        maxNbKeys = r[1] & 0x0F;
        keyType = (r[2] == 0x02) ? DF_KEY_AES : DF_KEY_DES;

        LOG(LogLevel::INFOS) << "Returns settings {0x" << std::hex << settings << std::dec << "(" << settings << ")} max nb Keys {" << maxNbKeys << "} key type {0x" << std::hex << keyType << std::dec << "(" << keyType << ")}";
    }

    unsigned char DESFireEV1STidSTRCommands::getKeyVersion(unsigned char keyno)
    {
        LOG(LogLevel::INFOS) << "Retrieving key version for key number {" << keyno << "}...";
        std::vector<unsigned char> command;
        command.push_back(keyno);
        std::vector<unsigned char> r = getSTidSTRReaderCardAdapter()->sendCommand(0x0064, command);

        EXCEPTION_ASSERT_WITH_LOG(r.size() >= 1, LibLogicalAccessException, "The response length should be at least 1-byte long");

        return r[0];
    }

    std::vector<unsigned char> DESFireEV1STidSTRCommands::getCardUID()
    {
        LOG(LogLevel::INFOS) << "Retrieving card uid...";
        std::vector<unsigned char> r = getSTidSTRReaderCardAdapter()->sendCommand(0x0051, std::vector<unsigned char>());

        return r;
    }

    void DESFireEV1STidSTRCommands::createStdDataFile(unsigned char fileno, EncryptionMode comSettings, DESFireAccessRights accessRights, unsigned int fileSize)
    {
        createStdDataFile(fileno, comSettings, accessRights, fileSize, 0x00);
    }

    void DESFireEV1STidSTRCommands::createStdDataFile(unsigned char fileno, EncryptionMode comSettings, DESFireAccessRights accessRights, unsigned int fileSize, unsigned short isoFID)
    {
        LOG(LogLevel::INFOS) << "Creating standard data file - file number {0x" << std::hex << fileno << std::dec << "(" << fileno << ")} encryption mode {0x"
            << std::hex << comSettings << std::dec << "(" << comSettings << ")} access right {0x"
            << std::hex << AccessRightsInMemory(accessRights) << std::dec << "(" << AccessRightsInMemory(accessRights) << ")} file size {" << fileSize << "}...";

        std::vector<unsigned char> command;
        short ar = AccessRightsInMemory(accessRights);

        command.push_back(static_cast<unsigned char>(fileno));
        if (isoFID != 0)
        {
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "isoFID parameter not available with this reader.");
        }
        command.push_back(static_cast<unsigned char>(comSettings));
        BufferHelper::setUShort(command, ar);
        command.push_back(static_cast<unsigned char>(fileSize & 0xff));
        command.push_back(static_cast<unsigned char>(static_cast<unsigned short>(fileSize & 0xff00) >> 8));
        command.push_back(static_cast<unsigned char>(static_cast<unsigned int>(fileSize & 0xff0000) >> 16));

        getSTidSTRReaderCardAdapter()->sendCommand(0x00CD, command);
    }

    void DESFireEV1STidSTRCommands::commitTransaction()
    {
        LOG(LogLevel::INFOS) << "Committing transaction...";

        getSTidSTRReaderCardAdapter()->sendCommand(0x00C7, std::vector<unsigned char>());
    }

    void DESFireEV1STidSTRCommands::abortTransaction()
    {
        LOG(LogLevel::INFOS) << "Aborting transaction...";

        getSTidSTRReaderCardAdapter()->sendCommand(0x00A7, std::vector<unsigned char>());
    }

    void DESFireEV1STidSTRCommands::createBackupFile(unsigned char fileno, EncryptionMode comSettings, DESFireAccessRights accessRights, unsigned int fileSize)
    {
        createBackupFile(fileno, comSettings, accessRights, fileSize, 0x00);
    }

    void DESFireEV1STidSTRCommands::createBackupFile(unsigned char fileno, EncryptionMode comSettings, DESFireAccessRights accessRights, unsigned int fileSize, unsigned short isoFID)
    {
        LOG(LogLevel::INFOS) << "Creating backup file - file number {0x" << std::hex << fileno << std::dec << "(" << fileno << ")} encryption mode {0x"
            << std::hex << comSettings << std::dec << "(" << comSettings << ")} access right {0x"
            << std::hex << AccessRightsInMemory(accessRights) << std::dec << "(" << AccessRightsInMemory(accessRights) << ")} file size {" << fileSize << "}...";

        std::vector<unsigned char> command;
        short ar = AccessRightsInMemory(accessRights);

        command.push_back(static_cast<unsigned char>(fileno));
        if (isoFID != 0)
        {
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "isoFID parameter not available with this reader.");
        }
        command.push_back(static_cast<unsigned char>(comSettings));
        BufferHelper::setUShort(command, ar);
        command.push_back(static_cast<unsigned char>(fileSize & 0xff));
        command.push_back(static_cast<unsigned char>(static_cast<unsigned short>(fileSize & 0xff00) >> 8));
        command.push_back(static_cast<unsigned char>(static_cast<unsigned int>(fileSize & 0xff0000) >> 16));

        getSTidSTRReaderCardAdapter()->sendCommand(0x00CB, command);
    }

    void DESFireEV1STidSTRCommands::createValueFile(unsigned char fileno, EncryptionMode comSettings, DESFireAccessRights accessRights, unsigned int lowerLimit, unsigned int upperLimit, unsigned int value, bool limitedCreditEnabled)
    {
        LOG(LogLevel::INFOS) << "Creating value file - file number {0x" << std::hex << fileno << std::dec << "(" << fileno << ")} encryption mode {0x"
            << std::hex << comSettings << std::dec << "(" << comSettings << ")} access right {0x"
            << std::hex << AccessRightsInMemory(accessRights) << std::dec << "(" << AccessRightsInMemory(accessRights) << ")} lower limit {" << lowerLimit << "} upper limit {" << upperLimit << "} value {0x"
            << std::hex << value << std::dec << "(" << value << ")}, limited credit enabled {" << limitedCreditEnabled << "}...";

        std::vector<unsigned char> command;
        short ar = AccessRightsInMemory(accessRights);

        command.push_back(fileno);
        command.push_back(static_cast<unsigned char>(comSettings));
        BufferHelper::setUShort(command, ar);
        BufferHelper::setInt32(command, lowerLimit);
        BufferHelper::setInt32(command, upperLimit);
        BufferHelper::setInt32(command, value);
        command.push_back(limitedCreditEnabled ? 0x01 : 0x00);

        getSTidSTRReaderCardAdapter()->sendCommand(0x00CC, command);
    }

    void DESFireEV1STidSTRCommands::createLinearRecordFile(unsigned char fileno, EncryptionMode comSettings, DESFireAccessRights accessRights, unsigned int fileSize, unsigned int maxNumberOfRecords)
    {
        createLinearRecordFile(fileno, comSettings, accessRights, fileSize, maxNumberOfRecords, 0x00);
    }

    void DESFireEV1STidSTRCommands::createLinearRecordFile(unsigned char fileno, EncryptionMode comSettings, DESFireAccessRights accessRights, unsigned int fileSize, unsigned int maxNumberOfRecords, unsigned short isoFID)
    {
        LOG(LogLevel::INFOS) << "Creating linear record file - file number {0x" << std::hex << fileno << std::dec << "(" << fileno << ")} encryption mode {0x"
            << std::hex << comSettings << std::dec << "(" << comSettings << ")} access right {0x"
            << std::hex << AccessRightsInMemory(accessRights) << std::dec << "(" << AccessRightsInMemory(accessRights) << ")} file size {" << fileSize << "} max number records {" << maxNumberOfRecords << "}...";

        std::vector<unsigned char> command;
        short ar = AccessRightsInMemory(accessRights);

        command.push_back(fileno);
        if (isoFID != 0)
        {
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "isoFID parameter not available with this reader.");
        }
        command.push_back(static_cast<unsigned char>(comSettings));
        BufferHelper::setUShort(command, ar);
        command.push_back(static_cast<unsigned char>(fileSize & 0xff));
        command.push_back(static_cast<unsigned char>(static_cast<unsigned short>(fileSize & 0xff00) >> 8));
        command.push_back(static_cast<unsigned char>(static_cast<unsigned int>(fileSize & 0xff0000) >> 16));
        command.push_back(static_cast<unsigned char>(maxNumberOfRecords & 0xff));
        command.push_back(static_cast<unsigned char>(static_cast<unsigned short>(maxNumberOfRecords & 0xff00) >> 8));
        command.push_back(static_cast<unsigned char>(static_cast<unsigned int>(maxNumberOfRecords & 0xff0000) >> 16));

        getSTidSTRReaderCardAdapter()->sendCommand(0x00C1, command);
    }

    void DESFireEV1STidSTRCommands::createCyclicRecordFile(unsigned char fileno, EncryptionMode comSettings, DESFireAccessRights accessRights, unsigned int fileSize, unsigned int maxNumberOfRecords)
    {
        createCyclicRecordFile(fileno, comSettings, accessRights, fileSize, maxNumberOfRecords, 0x00);
    }

    void DESFireEV1STidSTRCommands::createCyclicRecordFile(unsigned char fileno, EncryptionMode comSettings, DESFireAccessRights accessRights, unsigned int fileSize, unsigned int maxNumberOfRecords, unsigned short isoFID)
    {
        LOG(LogLevel::INFOS) << "Creating cyclic record file - file number {0x" << std::hex << fileno << std::dec << "(" << fileno << ")} encryption mode {0x"
            << std::hex << comSettings << std::dec << "(" << comSettings << ")} access right {0x"
            << std::hex << AccessRightsInMemory(accessRights) << std::dec << "(" << AccessRightsInMemory(accessRights) << ")} file size {"
            << fileSize << "} max number records {" << maxNumberOfRecords << "}...";

        std::vector<unsigned char> command;
        short ar = AccessRightsInMemory(accessRights);

        command.push_back(fileno);
        if (isoFID != 0)
        {
            command.push_back(static_cast<unsigned char>(static_cast<unsigned short>(isoFID & 0xff00) >> 8));
            command.push_back(static_cast<unsigned char>(isoFID & 0xff));
        }
        command.push_back(static_cast<unsigned char>(comSettings));
        BufferHelper::setUShort(command, ar);
        command.push_back(static_cast<unsigned char>(fileSize & 0xff));
        command.push_back(static_cast<unsigned char>(static_cast<unsigned short>(fileSize & 0xff00) >> 8));
        command.push_back(static_cast<unsigned char>(static_cast<unsigned int>(fileSize & 0xff0000) >> 16));
        command.push_back(static_cast<unsigned char>(maxNumberOfRecords & 0xff));
        command.push_back(static_cast<unsigned char>(static_cast<unsigned short>(maxNumberOfRecords & 0xff00) >> 8));
        command.push_back(static_cast<unsigned char>(static_cast<unsigned int>(maxNumberOfRecords & 0xff0000) >> 16));

        getSTidSTRReaderCardAdapter()->sendCommand(0x00C0, command);
    }

    void DESFireEV1STidSTRCommands::iso_selectFile(unsigned short /*fid*/)
    {
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Function not available with this reader.");
    }

    std::vector<unsigned char> DESFireEV1STidSTRCommands::iso_readRecords(unsigned short /*fid*/, unsigned char /*start_record*/, DESFireRecords /*record_number*/)
    {
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Function not available with this reader.");
        return std::vector<unsigned char>();
    }

    void DESFireEV1STidSTRCommands::iso_appendrecord(const std::vector<unsigned char>& /*data*/, unsigned short /*fid*/)
    {
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Function not available with this reader.");
    }

    std::vector<unsigned char> DESFireEV1STidSTRCommands::iso_getChallenge(unsigned int /*length*/)
    {
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Function not available with this reader.");
        return std::vector<unsigned char>();
    }

    void DESFireEV1STidSTRCommands::iso_externalAuthenticate(DESFireISOAlgorithm /*algorithm*/, bool /*isMasterCardKey*/, unsigned char /*keyno*/, const std::vector<unsigned char>& /*data*/)
    {
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Function not available with this reader.");
    }

    std::vector<unsigned char> DESFireEV1STidSTRCommands::iso_internalAuthenticate(DESFireISOAlgorithm /*algorithm*/, bool /*isMasterCardKey*/, unsigned char /*keyno*/, const std::vector<unsigned char>& /*RPCD2*/, unsigned int /*length*/)
    {
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Function not available with this reader.");
        return std::vector<unsigned char>();
    }

    void DESFireEV1STidSTRCommands::loadKey(std::shared_ptr<DESFireKey> key)
    {
        LOG(LogLevel::INFOS) << "Loading key from storage {" << key->serialize() << "}";
        std::shared_ptr<KeyStorage> key_storage = key->getKeyStorage();

        if (std::dynamic_pointer_cast<ComputerMemoryKeyStorage>(key_storage))
        {
            LOG(LogLevel::INFOS) << "Using computer memory key storage...";
            unsigned char* keydata = key->getData();
            loadKey(std::vector<unsigned char>(keydata, keydata + key->getLength()), key->getKeyDiversification() != NULL, true);
        }
        else if (std::dynamic_pointer_cast<ReaderMemoryKeyStorage>(key_storage))
        {
            LOG(LogLevel::INFOS) << "Using reader memory key storage... -> Doing nothing !";
        }
        else
        {
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "The key storage type is not supported for this card/reader.");
        }
    }

    void DESFireEV1STidSTRCommands::loadKey(std::vector<unsigned char> key, bool /*diversify*/, bool /*isVolatile*/)
    {
        LOG(LogLevel::INFOS) << "Loading key in reader memory... key size {" << key.size() << "}";
        EXCEPTION_ASSERT_WITH_LOG(key.size() == 16, LibLogicalAccessException, "The key length must be 16-byte long");

        std::vector<unsigned char> command;
        command.push_back(0x00); //(isVolatile ? 0x00 : 0x01);  Always use the Reader RAM Memory (0x00) (if want to write in EEPROM, use 0x01)
        command.insert(command.end(), key.begin(), key.end());
        command.push_back(0x00);	// Diversify, not supported yet.

        getSTidSTRReaderCardAdapter()->sendCommand(0x00D0, command);
    }

    void DESFireEV1STidSTRCommands::authenticate(STidKeyLocationType keylocation, unsigned char keyno, DESFireKeyType cryptoMethod, unsigned char keyindex)
    {
        LOG(LogLevel::INFOS) << "Authenticating from location... key location {" << DESFireEV1STidSTRCommands::STidKeyLocationTypeStr(keylocation) << "} key number {0x"
            << std::hex << keyno << std::dec << "(" << keyno << ")}, key type {" << DESFireKey::DESFireKeyTypeStr(cryptoMethod) << "}, key index {0x"
            << std::hex << keyindex << std::dec << "(" << keyindex << ")}";

        std::vector<unsigned char> command;
        command.push_back(static_cast<unsigned char>(keylocation));
        command.push_back(keyno);
        command.push_back((cryptoMethod == DF_KEY_AES) ? 0x02 : 0x00);
        command.push_back(keyindex);

        getSTidSTRReaderCardAdapter()->sendCommand(0x000B, command);
    }

    void DESFireEV1STidSTRCommands::authenticate(unsigned char keyno)
    {
		std::shared_ptr<DESFireKey> key = getDESFireChip()->getCrypto()->getKey(d_currentAid, keyno);
        authenticate(keyno, key);
    }

    void DESFireEV1STidSTRCommands::authenticate(unsigned char keyno, std::shared_ptr<DESFireKey> key)
    {
        LOG(LogLevel::INFOS) << "Authenticating... key number {0x" << std::hex << keyno << std::dec << "(" << keyno << ")}";

        std::shared_ptr<KeyStorage> key_storage = key->getKeyStorage();
        STidKeyLocationType keylocation = STID_DF_KEYLOC_RAM;
        unsigned char keyindex = 0x00;

        loadKey(key);	// Load key in the reader memory if needed (only when using Computer Memory Storage)

        if (std::dynamic_pointer_cast<ReaderMemoryKeyStorage>(key_storage))
        {
            LOG(LogLevel::INFOS) << "Retreving key index from reader memory key storage...";
            std::shared_ptr<ReaderMemoryKeyStorage> rmKs = std::dynamic_pointer_cast<ReaderMemoryKeyStorage>(key_storage);
            keylocation = STID_DF_KEYLOC_INDEXED;
            keyindex = rmKs->getKeySlot();
        }

        authenticate(keylocation, keyno, key->getKeyType(), keyindex);
    }

    void DESFireEV1STidSTRCommands::authenticateISO(unsigned char /*keyno*/, DESFireISOAlgorithm /*algorithm*/)
    {
        LOG(LogLevel::ERRORS) << "Function not available with this reader.";
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Function not available with this reader.");
    }

    void DESFireEV1STidSTRCommands::authenticateAES(unsigned char /*keyno*/)
    {
        LOG(LogLevel::ERRORS) << "Function not available with this reader.";
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Function not available with this reader.");
    }

    std::vector<unsigned char> DESFireEV1STidSTRCommands::readData(unsigned char fileno, unsigned int offset, unsigned int length, EncryptionMode mode)
    {
        LOG(LogLevel::INFOS) << "Reading data... file number {0x" << std::hex << fileno << std::dec << "(" << fileno << ")} offset {"
            << offset << "} length {" << length << "} encrypt mode {0x"
            << std::hex << mode << std::dec << "(" << mode << ")}";
        std::vector<unsigned char> command, result;

        size_t stidMaxDataSize = 1024; // New to version 1.2 of API (before was limited to 200)
        // STid STR reader doesn't support data larger than <stidMaxDataSize> bytes, we should split the command.
        for (size_t i = 0; i < length; i += stidMaxDataSize)
        {
            LOG(LogLevel::INFOS) << "Reading data from index {" << i << "} to " << (i + stidMaxDataSize - 1) << "...";
            command.clear();
            command.push_back(static_cast<unsigned char>(mode));
            command.push_back(fileno);
            size_t trunloffset = offset + i;
            size_t trunklength = ((length - i) > stidMaxDataSize) ? stidMaxDataSize : (length - i);
            command.insert(command.end(), (unsigned char*)&trunloffset, (unsigned char*)&trunloffset + 3);
            command.insert(command.end(), (unsigned char*)&trunklength, (unsigned char*)&trunklength + 3);

            std::vector<unsigned char> data = getSTidSTRReaderCardAdapter()->sendCommand(0x00BD, command);

			result.insert(result.end(), data.begin(), data.end());
        }
        LOG(LogLevel::INFOS) << "Returns data size {" << result.size() << "}";

        return result;
    }

    void DESFireEV1STidSTRCommands::writeData(unsigned char fileno, unsigned int offset, const std::vector<unsigned char>& data, EncryptionMode mode)
    {
        LOG(LogLevel::INFOS) << "Writing data... file number {0x"
            << std::hex << fileno << std::dec << "(" << fileno << ")} offset {" << offset << "} length {" << data.size() << "} encrypt mode {0x"
            << std::hex << mode << std::dec << "(" << mode << ")}";
        std::vector<unsigned char> command;

        size_t stidMaxDataSize = 1024; // New to version 1.2 of API (before was limited to 200)
        // STid STR reader doesn't support data larger than <stidMaxDataSize> bytes, we should split the command.
        for (size_t i = 0; i < data.size(); i += stidMaxDataSize)
        {
            LOG(LogLevel::INFOS) << "Writing data from index {" << i << "} to {" << i + stidMaxDataSize - 1 << "}...";
            command.clear();
            command.push_back(static_cast<unsigned char>(mode));
            command.push_back(fileno);
            size_t trunloffset = offset + i;
            size_t trunklength = ((data.size() - i) > stidMaxDataSize) ? stidMaxDataSize : (data.size() - i);
            command.insert(command.end(), (unsigned char*)&trunloffset, (unsigned char*)&trunloffset + 3);
            command.insert(command.end(), (unsigned char*)&trunklength, (unsigned char*)&trunklength + 3);
            command.insert(command.end(), data.begin() + i, data.begin() + i + trunklength);

            getSTidSTRReaderCardAdapter()->sendCommand(0x003D, command);
        }
    }

    void DESFireEV1STidSTRCommands::credit(unsigned char fileno, unsigned int value, EncryptionMode mode)
    {
        LOG(LogLevel::INFOS) << "Increasing value file... file number {0x"
            << std::hex << fileno << std::dec << "(" << fileno << ")} value {0x"
            << std::hex << value << std::dec << "(" << value << ")} encrypt mode {0x"
            << std::hex << mode << std::dec << "(" << mode << ")}";
        std::vector<unsigned char> command;
        command.push_back(static_cast<unsigned char>(mode));
        command.push_back(fileno);
        BufferHelper::setUInt32(command, value);

        getSTidSTRReaderCardAdapter()->sendCommand(0x000C, command);
    }

    void DESFireEV1STidSTRCommands::debit(unsigned char fileno, unsigned int value, EncryptionMode mode)
    {
        LOG(LogLevel::INFOS) << "Decreasing value file... file number {0x"
            << std::hex << fileno << std::dec << "(" << fileno << ")} value {0x"
            << std::hex << value << std::dec << "(" << value << ")} encrypt mode {0x"
            << std::hex << mode << std::dec << "(" << mode << ")}";
        std::vector<unsigned char> command;
        command.push_back(static_cast<unsigned char>(mode));
        command.push_back(fileno);
        BufferHelper::setUInt32(command, value);

        getSTidSTRReaderCardAdapter()->sendCommand(0x00DC, command);
    }

    void DESFireEV1STidSTRCommands::limitedCredit(unsigned char fileno, unsigned int value, EncryptionMode mode)
    {
        LOG(LogLevel::INFOS) << "Increasing value file with limited amount... file number {0x"
            << std::hex << fileno << std::dec << "(" << fileno << ")} value {0x"
            << std::hex << value << std::dec << "(" << value << ")} encrypt mode {0x"
            << std::hex << mode << std::dec << "(" << mode << ")}";
        std::vector<unsigned char> command;
        command.push_back(static_cast<unsigned char>(mode));
        command.push_back(fileno);
        BufferHelper::setUInt32(command, value);

        getSTidSTRReaderCardAdapter()->sendCommand(0x001C, command);
    }

    void DESFireEV1STidSTRCommands::writeRecord(unsigned char fileno, unsigned int offset, const std::vector<unsigned char>& data, EncryptionMode mode)
    {
        LOG(LogLevel::INFOS) << "Writing record... file number {0x"
            << std::hex << fileno << std::dec << "(" << fileno << ")} offset {" << offset << "} length {" << data.size() << "} encrypt mode {0x"
            << std::hex << mode << std::dec << "(" << mode << ")}";
        std::vector<unsigned char> command;

        size_t stidMaxDataSize = 1024; // New to version 1.2 of API (before was limited to 200)
        // STid STR reader doesn't support data larger than <stidMaxDataSize> bytes, we should split the command.
        for (size_t i = 0; i < data.size(); i += stidMaxDataSize)
        {
            LOG(LogLevel::INFOS) << "Writing data from index {" << i << "} to {" << i + stidMaxDataSize - 1 << "}...";
            command.clear();
            command.push_back(static_cast<unsigned char>(mode));
            command.push_back(fileno);
            size_t trunloffset = offset + i;
            size_t trunklength = ((data.size() - i) > stidMaxDataSize) ? stidMaxDataSize : (data.size() - i);
            command.insert(command.end(), (unsigned char*)&trunloffset, (unsigned char*)&trunloffset + 3);
            command.insert(command.end(), (unsigned char*)&trunklength, (unsigned char*)&trunklength + 3);
            command.insert(command.end(), data.begin() + i, data.begin() + i + trunklength);

            getSTidSTRReaderCardAdapter()->sendCommand(0x003B, command);
        }
    }

    std::vector<unsigned char> DESFireEV1STidSTRCommands::readRecords(unsigned char fileno, unsigned int offset, unsigned int nbrecords, EncryptionMode mode)
    {
        LOG(LogLevel::INFOS) << "Reading record... file number {0x"
            << std::hex << fileno << std::dec << "(" << fileno << ")} offset {" << offset << "} nb records {" << nbrecords << "} encrypt mode {0x"
            << std::hex << mode << std::dec << "(" << mode << ")}";
        //size_t stidMaxDataSize = 1024; // New to version 1.2 of API (before was limited to 200)
        // STid STR reader doesn't support data larger than <stidMaxDataSize> bytes, a record cannot exceed this size.

        std::vector<unsigned char> command;
        command.push_back(static_cast<unsigned char>(mode));
        command.push_back(fileno);
        command.insert(command.end(), (unsigned char*)&offset, (unsigned char*)&offset + 3);
        command.insert(command.end(), (unsigned char*)&nbrecords, (unsigned char*)&nbrecords + 3);

        std::vector<unsigned char> result = getSTidSTRReaderCardAdapter()->sendCommand(0x00BB, command);

        LOG(LogLevel::INFOS) << "Returns size {" << result.size() << "}";
        return result;
    }

    void DESFireEV1STidSTRCommands::clearRecordFile(unsigned char fileno)
    {
        LOG(LogLevel::INFOS) << "Clearing record... file number {0x" << std::hex << fileno << std::dec << "(" << fileno << ")}";

        std::vector<unsigned char> command;
        command.push_back(fileno);

        getSTidSTRReaderCardAdapter()->sendCommand(0x00EB, command);
    }

    void DESFireEV1STidSTRCommands::changeFileSettings(unsigned char fileno, EncryptionMode comSettings, DESFireAccessRights accessRights, bool plain)
    {
        LOG(LogLevel::INFOS) << "Changing file settings... file number {0x"
            << std::hex << fileno << std::dec << "(" << fileno << ")} com settings file number {0x"
            << std::hex << comSettings << std::dec << "(" << comSettings << ")} access rights file number {0x"
            << std::hex << AccessRightsInMemory(accessRights) << std::dec << "(" << AccessRightsInMemory(accessRights) << ")} plain {" << plain << "}";
        std::vector<unsigned char> command;
        short ar = AccessRightsInMemory(accessRights);

        command.push_back(fileno);
        command.push_back(plain ? 0x00 : 0x01);
        command.push_back(static_cast<unsigned char>(comSettings));
        BufferHelper::setUShort(command, ar);

        getSTidSTRReaderCardAdapter()->sendCommand(0x005F, command);
    }

    void DESFireEV1STidSTRCommands::deleteFile(unsigned char fileno)
    {
        LOG(LogLevel::INFOS) << "Deleting file... file number {0x" << std::hex << fileno << std::dec << "(" << fileno << ")}";

        std::vector<unsigned char> command;
        command.push_back(fileno);

        getSTidSTRReaderCardAdapter()->sendCommand(0x00DF, command);
    }

    void DESFireEV1STidSTRCommands::changeKeySettings(DESFireKeySettings settings)
    {
        LOG(LogLevel::INFOS) << "Changing key settings... settings {0x" << std::hex << settings << std::dec << "(" << settings << ")}";
        std::vector<unsigned char> command;
        command.push_back(static_cast<unsigned char>(settings));

        getSTidSTRReaderCardAdapter()->sendCommand(0x0054, command);
    }

    void DESFireEV1STidSTRCommands::changeKey(unsigned char keyno, std::shared_ptr<DESFireKey> key)
    {
        LOG(LogLevel::INFOS) << "Changing key... key number {0x" << std::hex << keyno << std::dec << "(" << keyno << ")} new key {" << key->serialize() << "}";
        // Only change the key if new key and old key are not the same.
		std::shared_ptr<DESFireKey> oldKey = getDESFireChip()->getCrypto()->getKey(d_currentAid, keyno);

        LOG(LogLevel::INFOS) << "Old key {" << oldKey->serialize() << "}";

        if (key != oldKey)
        {
            // Old key and new key use reader memory. Use change keyindex instead.
            if (key->getKeyStorage()->getType() == KST_READER_MEMORY && oldKey->getKeyStorage()->getType() == KST_READER_MEMORY)
            {
                LOG(LogLevel::INFOS) << "Changing key by using reader EEPROM INDEXED memory key storage...";
                changeKeyIndex(keyno, key->getKeyType(), key->getKeyVersion(), std::dynamic_pointer_cast<ReaderMemoryKeyStorage>(key->getKeyStorage())->getKeySlot(), std::dynamic_pointer_cast<ReaderMemoryKeyStorage>(oldKey->getKeyStorage())->getKeySlot());
            }
            else
            {
                LOG(LogLevel::INFOS) << "Changing key directly...";
                std::vector<unsigned char> command;
                command.push_back(keyno);
                command.push_back((key->getKeyType() == DF_KEY_AES) ? 0x02 : 0x00);
                if (key->getKeyType() == DF_KEY_AES)
                {
                    command.push_back(key->getKeyVersion());
                }

                unsigned char* keydata = key->getData();
                command.insert(command.end(), keydata, keydata + key->getLength());
                keydata = oldKey->getData();
                command.insert(command.end(), keydata, keydata + oldKey->getLength());

                getSTidSTRReaderCardAdapter()->sendCommand(0x00C4, command);
            }
        }
        else
        {
            LOG(LogLevel::INFOS) << "Key was already loaded. Doing nothing.";
        }
    }

    void DESFireEV1STidSTRCommands::changeKeyIndex(unsigned char keyno, DESFireKeyType cryptoMethod, unsigned char keyversion, unsigned char newkeyindex, unsigned char oldkeyindex)
    {
        LOG(LogLevel::INFOS) << "Changing key index... key number {0x" << std::hex << keyno << std::dec << "(" << keyno << ")} crypto method {0x"
            << std::hex << cryptoMethod << std::dec << "(" << cryptoMethod << ")} key version {0x" << std::hex << keyversion << std::dec << "(" << keyversion << ")} new key index {0x"
            << std::hex << newkeyindex << std::dec << "(" << newkeyindex << ")} old key index {0x" << std::hex << oldkeyindex << std::dec << "(" << oldkeyindex << ")}";

        std::vector<unsigned char> command;
        command.push_back(keyno);
        command.push_back((cryptoMethod == DF_KEY_AES) ? 0x02 : 0x00);
        if (cryptoMethod == DF_KEY_AES)
        {
            command.push_back(keyversion);
        }
        command.push_back(newkeyindex);
        command.push_back(oldkeyindex);
        command.push_back(0x00); // RFU

        getSTidSTRReaderCardAdapter()->sendCommand(0x00C5, command);
    }

	DESFireCommands::DESFireCardVersion DESFireEV1STidSTRCommands::getVersion()
    {
        LOG(LogLevel::INFOS) << "Retrieving version...";
        std::vector<unsigned char> result = getSTidSTRReaderCardAdapter()->sendCommand(0x0060, std::vector<unsigned char>());
		DESFireCommands::DESFireCardVersion dataVersion;
        EXCEPTION_ASSERT_WITH_LOG(result.size() >= 28, LibLogicalAccessException, "The response length should be at least 28-byte long");

        memcpy(reinterpret_cast<char*>(&dataVersion), &result[0], 28);
		return dataVersion;
    }

    std::vector<unsigned int> DESFireEV1STidSTRCommands::getApplicationIDs()
    {
        LOG(LogLevel::INFOS) << "Retrieving all application ids...";
        std::vector<unsigned int> aids;

        std::vector<unsigned char> result = getSTidSTRReaderCardAdapter()->sendCommand(0x006A, std::vector<unsigned char>());

        unsigned nbaids = result[0];
        for (size_t i = 0; i < nbaids; ++i)
        {
            std::vector<unsigned char> aid(3);
            std::copy(result.begin() + 1 + (i * 3), result.begin() + 1 + (i * 3) + 3, aid.begin());
            aids.push_back(DESFireLocation::convertAidToUInt(aid));
        }

        for (std::vector<unsigned int>::iterator it = aids.begin(); it != aids.end(); ++it)
        {
            LOG(LogLevel::INFOS) << "Processing application id {0x" << std::hex << *it << std::dec << "(" << *it << ")}...";
        }

        return aids;
    }

    void DESFireEV1STidSTRCommands::erase()
    {
        LOG(LogLevel::INFOS) << "Erasing card...";
        getSTidSTRReaderCardAdapter()->sendCommand(0x00FC, std::vector<unsigned char>());
    }

    std::vector<unsigned char> DESFireEV1STidSTRCommands::getFileIDs()
    {
        LOG(LogLevel::INFOS) << "Retrieving all files ids...";
        std::vector<unsigned char> result = getSTidSTRReaderCardAdapter()->sendCommand(0x006F, std::vector<unsigned char>());

        unsigned char nbfiles = result[0];

        std::vector<unsigned char> files;
        for (size_t i = 1; i <= nbfiles; ++i)
        {
            files.push_back(result[i]);
        }

        for (std::vector<unsigned char>::iterator it = files.begin(); it != files.end(); ++it)
        {
            LOG(LogLevel::INFOS) << "Processing file id {0x" << std::hex << *it << std::dec << "(" << *it << ")}...";
        }

        return files;
    }

	DESFireCommands::FileSetting DESFireEV1STidSTRCommands::getFileSettings(unsigned char fileno)
    {
        LOG(LogLevel::INFOS) << "Retrieving file settings for file number {0x" << std::hex << fileno << std::dec << "(" << fileno << ")}";

		FileSetting fileSetting;
        std::vector<unsigned char> command;
        command.push_back(fileno);

        std::vector<unsigned char> result = getSTidSTRReaderCardAdapter()->sendCommand(0x00F5, command);
        memcpy(&fileSetting, &result[0], result.size());
		return fileSetting;
    }

    void DESFireEV1STidSTRCommands::getValue(unsigned char fileno, EncryptionMode mode, unsigned int& value)
    {
        LOG(LogLevel::INFOS) << "Retrieving value for file number {0x" << std::hex << fileno << std::dec << "(" << fileno << ")} encrypt mode {0x" << std::hex << mode << std::dec << "(" << mode << ")}";

        std::vector<unsigned char> command;
        command.push_back(static_cast<unsigned char>(mode));
        command.push_back(fileno);

        std::vector<unsigned char> result = getSTidSTRReaderCardAdapter()->sendCommand(0x00F5, command);

        if (result.size() >= 4)
        {
            size_t offset = 0;
            value = BufferHelper::getUInt32(result, offset);
            LOG(LogLevel::INFOS) << "Result value {0x" << std::hex << value << std::dec << "(" << value << ")}";
        }
    }

    void DESFireEV1STidSTRCommands::iso_selectApplication(std::vector<unsigned char> /*isoaid*/)
    {
        LOG(LogLevel::ERRORS) << "Function not available with this reader.";
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Function not available with this reader.");
    }

    void DESFireEV1STidSTRCommands::setConfiguration(bool formatCardEnabled, bool randomIdEnabled)
    {
        std::vector<unsigned char> command;
        command.push_back(0x00);
        command.push_back(0x01);
        command.push_back(0x00 | ((formatCardEnabled) ? 0x00 : 0x01) | ((randomIdEnabled) ? 0x02 : 0x00));

        getSTidSTRReaderCardAdapter()->sendCommand(0x005C, command);
    }

    void DESFireEV1STidSTRCommands::setConfiguration(std::shared_ptr<DESFireKey> defaultKey)
    {
        if (defaultKey->getLength() < 24)
        {
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "The default key length must be 24-byte long.");
        }

        std::vector<unsigned char> command;
        command.push_back(0x01);
        command.push_back(0x19);
        unsigned char* keydata = defaultKey->getData();
        command.insert(command.end(), keydata, keydata + 24);
        command.push_back(defaultKey->getKeyVersion());

        getSTidSTRReaderCardAdapter()->sendCommand(0x005C, command);
    }

    void DESFireEV1STidSTRCommands::setConfiguration(const std::vector<unsigned char>& ats)
    {
        std::vector<unsigned char> command;
        command.push_back(0x02);
        command.push_back(static_cast<unsigned char>(ats.size()));
        command.insert(command.end(), ats.begin(), ats.end());

        getSTidSTRReaderCardAdapter()->sendCommand(0x005C, command);
    }

    std::string DESFireEV1STidSTRCommands::STidKeyLocationTypeStr(STidKeyLocationType t)
    {
        switch (t)
        {
        case STID_DF_KEYLOC_RAM: return "STID_DF_KEYLOC_RAM";
        case STID_DF_KEYLOC_EEPROM: return "STID_DF_KEYLOC_EEPROM";
        case STID_DF_KEYLOC_INDEXED: return "STID_DF_KEYLOC_INDEXED";
        default: return "Unknown";
        }
    }
}