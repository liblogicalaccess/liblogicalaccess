/**
 * \file desfireiso7816commands.cpp
 * \author Maxime C. <maxime@leosac.com>
 * \brief DESFire ISO7816 commands.
 */

#include <logicalaccess/plugins/readers/iso7816/commands/desfireiso7816commands.hpp>
#include <logicalaccess/plugins/cards/desfire/desfirechip.hpp>
#include <logicalaccess/plugins/readers/iso7816/commands/samav1iso7816commands.hpp>
#include <logicalaccess/cards/samkeystorage.hpp>
#include <logicalaccess/plugins/cards/desfire/nxpkeydiversification.hpp>
#include <logicalaccess/plugins/cards/desfire/nxpav1keydiversification.hpp>
#include <logicalaccess/plugins/cards/desfire/nxpav2keydiversification.hpp>
#include <logicalaccess/plugins/cards/samav/samav2commands.hpp>
#include <logicalaccess/plugins/llacommon/settings.hpp>
#include <logicalaccess/cards/computermemorykeystorage.hpp>
#include <logicalaccess/dynlibrary/librarymanager.hpp>

namespace logicalaccess
{
DESFireISO7816Commands::DESFireISO7816Commands()
    : DESFireCommands(CMD_DESFIREISO7816)
{
}

DESFireISO7816Commands::DESFireISO7816Commands(std::string ct)
    : DESFireCommands(ct)
{
}

DESFireISO7816Commands::~DESFireISO7816Commands()
{
}

void DESFireISO7816Commands::erase()
{
    auto result = transmit(DF_INS_FORMAT_PICC);
    if (result.getSW1() != 0x91 || result.getSW2() != 0x00)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Erase command failed.");
}

DESFireCommands::DESFireCardVersion DESFireISO7816Commands::getVersion()
{
    DESFireCardVersion dataVersion = DESFireCardVersion();

    auto result = transmit(DF_INS_GET_VERSION);
    if (result.getData().size() == 7)
    {
        if (result.getSW2() == DF_INS_ADDITIONAL_FRAME)
        {
            memcpy(&dataVersion, &result.getData()[0], 7);
            result = transmit(DF_INS_ADDITIONAL_FRAME);
            if (result.getData().size() == 7)
            {
                if (result.getSW2() == DF_INS_ADDITIONAL_FRAME)
                {
                    memcpy(reinterpret_cast<char *>(&dataVersion) + 7,
                           &result.getData()[0], 7);
                    result = transmit(DF_INS_ADDITIONAL_FRAME);
                    if (result.getData().size() == 14)
                    {
                        if (result.getSW2() == 0x00)
                        {
                            memcpy(reinterpret_cast<char *>(&dataVersion) + 14,
                                   &result.getData()[0], 14);
                        }
                    }
                }
            }
        }
    }
    return dataVersion;
}

void DESFireISO7816Commands::selectApplication(unsigned int aid)
{
    ByteVector command; //, samaid;
    DESFireLocation::convertUIntToAid(aid, command);

    DESFireISO7816Commands::transmit(DF_INS_SELECT_APPLICATION, command);

    /*
     * We directly select the keyentry to use so no need to select the app
      if (getSAMChip())
     {
     LOG(LogLevel::INFOS) << "SelectApplication on SAM chip...");
     DESFireLocation::convertUIntToAid(aid, samaid);
     std::reverse(samaid.begin(), samaid.end());
     if (getSAMChip()->getCardType() == "SAM_AV1")
     std::dynamic_pointer_cast<SAMCommands<KeyEntryAV1Information, SETAV1>
     >(getSAMChip()->getCommands())->selectApplication(samaid);
     else if (getSAMChip()->getCardType() == "SAM_AV2")
     std::dynamic_pointer_cast<SAMCommands<KeyEntryAV2Information, SETAV2>
     >(getSAMChip()->getCommands())->selectApplication(samaid);
     }*/

    getDESFireChip()->getCrypto()->selectApplication(aid);
}

void DESFireISO7816Commands::createApplication(unsigned int aid,
                                               DESFireKeySettings settings,
                                               unsigned char maxNbKeys)
{
    ByteVector command;

    DESFireLocation::convertUIntToAid(aid, command);
    command.push_back(static_cast<unsigned char>(settings));
    command.push_back(static_cast<unsigned char>(maxNbKeys));

    transmit(DF_INS_CREATE_APPLICATION, command, sizeof(command));
}

void DESFireISO7816Commands::deleteApplication(unsigned int aid)
{
    ByteVector command;

    DESFireLocation::convertUIntToAid(aid, command);

    transmit(DF_INS_DELETE_APPLICATION, command, sizeof(command));
}

std::vector<unsigned int> DESFireISO7816Commands::getApplicationIDs()
{
    std::vector<unsigned int> aids;
    auto result = transmit(DF_INS_GET_APPLICATION_IDS);

    while (result.getSW2() == DF_INS_ADDITIONAL_FRAME)
    {
        for (size_t i = 0; i < result.getData().size(); i += 3)
        {
            ByteVector aid(result.getData().begin() + i, result.getData().begin() + i + 3);
            aids.push_back(DESFireLocation::convertAidToUInt(aid));
        }

        result = transmit(DF_INS_ADDITIONAL_FRAME);
    }

    for (size_t i = 0; i < result.getData().size(); i += 3)
    {
        ByteVector aid(result.getData().begin() + i, result.getData().begin() + i + 3);
        aids.push_back(DESFireLocation::convertAidToUInt(aid));
    }

    return aids;
}

ByteVector
DESFireISO7816Commands::getChangeKeySAMCryptogram(unsigned char keyno,
                                                  std::shared_ptr<DESFireKey> key,
                                                  bool changeKeyEV2,
                                                  unsigned char keysetno) const
{
    std::shared_ptr<SAMKeyStorage> samsks =
        std::dynamic_pointer_cast<SAMKeyStorage>(key->getKeyStorage());

    if (samsks && !getSAMChip())
        THROW_EXCEPTION_WITH_LOG(
            LibLogicalAccessException,
            "SAMKeyStorage set on the key but no SAM reader has been set.");

    std::shared_ptr<SAMCommands<KeyEntryAV1Information, SETAV1>> samav1commands =
        std::dynamic_pointer_cast<SAMCommands<KeyEntryAV1Information, SETAV1>>(
            getSAMChip()->getCommands());
    std::shared_ptr<SAMCommands<KeyEntryAV2Information, SETAV2>> samav2commands =
        std::dynamic_pointer_cast<SAMCommands<KeyEntryAV2Information, SETAV2>>(
            getSAMChip()->getCommands());

    std::shared_ptr<DESFireCrypto> crypto = getDESFireChip()->getCrypto();
    std::shared_ptr<DESFireKey> oldkey =
        std::dynamic_pointer_cast<DESFireKey>(crypto->getKey(0, keyno));

    ChangeKeyInfo samck;
    memset(&samck, 0x00, sizeof(samck));
    samck.currentKeySlotNo = 0;
    samck.isMasterKey      = (crypto->d_currentAid == 0 && keyno == 0x00);
    samck.newKeySlotNo     = samsks->getKeySlot();
    samck.newKeySlotV      = key->getKeyVersion();
    samck.desfireNumber    = keyno;
    samck.useChangeKeyEV2  = changeKeyEV2;
    samck.keysetNumber     = keysetno;

    if (oldkey && std::dynamic_pointer_cast<SAMKeyStorage>(oldkey->getKeyStorage()))
    {
        const std::shared_ptr<SAMKeyStorage> oldsamks =
            std::dynamic_pointer_cast<SAMKeyStorage>(oldkey->getKeyStorage());
        samck.currentKeySlotNo = oldsamks->getKeySlot();
        samck.currentKeySlotV  = oldkey->getKeyVersion();
        
        samck.oldKeyInvolvement = (keyno == 0xE);
    }
    else
    {
        if (crypto->d_currentKeyNo != keyno)
        {
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                "Current key required on SAM to change the key.");
        }
        samck.oldKeyInvolvement = true;
    }

    ByteVector diversify;
    if (key->getKeyDiversification())
        key->getKeyDiversification()->initDiversification(
            crypto->getIdentifier(), crypto->d_currentAid, key, keyno, diversify);
    if (oldkey->getKeyDiversification())
        oldkey->getKeyDiversification()->initDiversification(
            crypto->getIdentifier(), crypto->d_currentAid, oldkey, keyno, diversify);

    ChangeKeyDiversification keyDiv;
    memset(&keyDiv, 0x00, sizeof(keyDiv));
    if (std::dynamic_pointer_cast<NXPKeyDiversification>(key->getKeyDiversification()) ||
        std::dynamic_pointer_cast<NXPKeyDiversification>(oldkey->getKeyDiversification()))
    {
        ByteVector diversifyNew, diversifyOld;
        std::shared_ptr<KeyDiversification> nxpdiv    = key->getKeyDiversification();
        std::shared_ptr<KeyDiversification> oldnxpdiv = oldkey->getKeyDiversification();

        if (nxpdiv)
        {
            nxpdiv->initDiversification(crypto->getIdentifier(), crypto->d_currentAid,
                                        key, keyno, diversifyNew);
            keyDiv.diversifyNew = 0x01;
            keyDiv.divInput     = new unsigned char[diversifyNew.size()];
            memcpy(keyDiv.divInput, &diversifyNew[0], diversifyNew.size());
            keyDiv.divInputSize = static_cast<unsigned char>(diversifyNew.size());
        }

        if (oldnxpdiv)
        {
            oldnxpdiv->initDiversification(crypto->getIdentifier(), crypto->d_currentAid,
                                           key, keyno, diversifyOld);
            if (!keyDiv.divInput)
            {
                keyDiv.divInput = new unsigned char[diversifyOld.size()];
                memcpy(keyDiv.divInput, &diversifyOld[0], diversifyOld.size());
                keyDiv.divInputSize = static_cast<unsigned char>(diversifyOld.size());
            }

            if (nxpdiv && diversifyNew.size() != 0 &&
                !equal(diversifyNew.begin(), diversifyNew.end(), diversifyOld.begin()) &&
                typeid(*nxpdiv) != typeid(*oldnxpdiv))
                THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                         "Current and New Key should have the same "
                                         "system identifier and same NXP Div type.");

            keyDiv.diversifyCurrent = 0x01;
        }

        if (std::dynamic_pointer_cast<NXPAV2KeyDiversification>(nxpdiv) ||
            std::dynamic_pointer_cast<NXPAV2KeyDiversification>(oldnxpdiv))
            keyDiv.divType = SAMAV2;
        else
            keyDiv.divType = SAMAV1;
    }

    if (crypto->d_auth_method != CM_LEGACY)
    {
        // AES has keep IV option, need to load current IV
        if (samav1commands)
            samav1commands->loadInitVector(crypto->d_lastIV);
        else
            samav2commands->loadInitVector(crypto->d_lastIV);
    }

    ByteVector ret;
    if (samav1commands)
        ret = samav1commands->changeKeyPICC(samck, keyDiv);
    else
        ret = samav2commands->changeKeyPICC(samck, keyDiv);

    if (keyDiv.divInput != nullptr)
        delete[] keyDiv.divInput;

    crypto->d_lastIV.clear();
    crypto->d_lastIV.resize(crypto->d_cipher->getBlockSize());
    copy(ret.end() - crypto->d_cipher->getBlockSize(), ret.end(),
         crypto->d_lastIV.begin());
    return ret;
}

uint8_t DESFireISO7816Commands::getKeyVersion(uint8_t keyno)
{
    ByteVector command;
    command.push_back(keyno);
    auto result = transmit(DF_INS_GET_KEY_VERSION, command).getData();
    if (result.size() < 1)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "Invalid response for getKeyVersion.");
    return result[0];
}

ByteVector DESFireISO7816Commands::getKeyInformations(std::shared_ptr<DESFireKey> key,
                                                      uint8_t keyno) const
{
    auto crypto        = getDESFireChip()->getCrypto();
    auto samKeyStorage = std::dynamic_pointer_cast<SAMKeyStorage>(key->getKeyStorage());

    ByteVector diversify;
    if (key->getKeyDiversification())
        key->getKeyDiversification()->initDiversification(
            crypto->getIdentifier(), crypto->d_currentAid, key, keyno, diversify);

    // get key value from SAM - But what to do if the initDiversification need the key ?
    if (samKeyStorage && samKeyStorage->getDumpKey())
        getKeyFromSAM(key, diversify);

    return diversify;
}

bool DESFireISO7816Commands::checkChangeKeySAMKeyStorage(
    unsigned char keyno, std::shared_ptr<DESFireKey> oldkey,
    std::shared_ptr<DESFireKey> key)
{
    std::shared_ptr<DESFireCrypto> crypto = getDESFireChip()->getCrypto();
    auto oldSamKeyStorage =
        std::dynamic_pointer_cast<SAMKeyStorage>(oldkey->getKeyStorage());
    auto newSamKeyStorage =
        std::dynamic_pointer_cast<SAMKeyStorage>(key->getKeyStorage());
    if ((oldSamKeyStorage && !oldSamKeyStorage->getDumpKey()) && !newSamKeyStorage)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "Both keys need to be set in the SAM.");

    return (newSamKeyStorage && !newSamKeyStorage->getDumpKey());
}

void DESFireISO7816Commands::changeKey(unsigned char keyno,
                                       std::shared_ptr<DESFireKey> newkey)
{
    std::shared_ptr<DESFireCrypto> crypto = getDESFireChip()->getCrypto();
    ByteVector cryptogram;

    std::shared_ptr<DESFireKey> key = std::make_shared<DESFireKey>(*newkey);
    auto oldkey                     = crypto->getKey(0, keyno);

    bool samChangeKey = checkChangeKeySAMKeyStorage(keyno, oldkey, key);

    auto oldKeyDiversify = getKeyInformations(oldkey, keyno);
    auto newKeyDiversify = getKeyInformations(key, keyno);

    if (samChangeKey)
    {
        cryptogram = getChangeKeySAMCryptogram(keyno, key);
    }
    else
    {
        cryptogram = crypto->changeKey_PICC(keyno, oldKeyDiversify, key, newKeyDiversify);
    }

    ByteVector command;
    command.push_back(keyno);
    if (cryptogram.size() > 0)
    {
        command.insert(command.end(), cryptogram.begin(), cryptogram.end());
        transmit(DF_INS_CHANGE_KEY, command);
        crypto->setKey(crypto->d_currentAid, 0, keyno, newkey);
    }
}

ByteVector DESFireISO7816Commands::getFileIDs()
{
    ByteVector result = transmit(DF_INS_GET_FILE_IDS).getData();
    ByteVector files;

    for (size_t i = 0; i < result.size(); ++i)
    {
        files.push_back(result[i]);
    }

    return files;
}

void DESFireISO7816Commands::getKeySettings(DESFireKeySettings &settings,
                                            unsigned char &maxNbKeys)
{
    ByteVector result = transmit(DF_INS_GET_KEY_SETTINGS).getData();

    settings  = static_cast<DESFireKeySettings>(result[0]);
    maxNbKeys = result[1];
}

void DESFireISO7816Commands::changeKeySettings(DESFireKeySettings settings)
{
    unsigned char command[1];
    command[0]            = static_cast<unsigned char>(settings);
    ByteVector cryptogram = getDESFireChip()->getCrypto()->desfireEncrypt(
        ByteVector(command, command + sizeof(command)));
    transmit(DF_INS_CHANGE_KEY_SETTINGS, cryptogram);
}

DESFireCommands::FileSetting DESFireISO7816Commands::getFileSettings(unsigned char fileno)
{
    FileSetting fileSetting;
    ByteVector command;
    command.push_back(fileno);

    ByteVector result = transmit(DF_INS_GET_FILE_SETTINGS, command).getData();
    memcpy(&fileSetting, &result[0], result.size());
    return fileSetting;
}

ByteVector DESFireISO7816Commands::handleReadData(unsigned char err,
                                                  const ByteVector &firstMsg,
                                                  unsigned int length,
                                                  EncryptionMode mode)
{
    std::shared_ptr<DESFireCrypto> crypto = getDESFireChip()->getCrypto();
    ByteVector ret, data;

    if ((err == DF_INS_ADDITIONAL_FRAME || err == 0x00))
    {
        if (mode == CM_ENCRYPT)
        {
            crypto->initBuf();
            crypto->appendDecipherData(firstMsg);
        }
        else
        {
            ret = data = firstMsg;
            if (mode == CM_MAC)
            {
                crypto->initBuf();
            }
        }
    }

    while (err == DF_INS_ADDITIONAL_FRAME)
    {
        if (mode == CM_MAC)
        {
            if (!crypto->verifyMAC(false, data))
            {
                THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                         "MAC data doesn't match.");
            }
        }

        auto result = transmit(DF_INS_ADDITIONAL_FRAME);
        err         = result.getSW2();
        data        = result.getData();

        if (mode == CM_ENCRYPT)
        {
            crypto->appendDecipherData(data);
        }
        else
        {
            ret.insert(ret.end(), data.begin(), data.end());
        }
    }

    switch (mode)
    {
    case CM_PLAIN: {
    }
    break;

    case CM_MAC:
    {
        if (!crypto->verifyMAC(true, data))
        {
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                     "MAC data doesn't match.");
        }
        ret.resize(ret.size() - 4);
    }
    break;
    case CM_ENCRYPT: { ret = crypto->desfireDecrypt(length);
    }
    break;
    case CM_UNKNOWN: {
    }
    break;
    }

    return ret;
}

void DESFireISO7816Commands::handleWriteData(unsigned char cmd,
                                             const ByteVector &parameters,
                                             const ByteVector &data, EncryptionMode mode)
{
    std::shared_ptr<DESFireCrypto> crypto = getDESFireChip()->getCrypto();

    crypto->initBuf();

    ByteVector cmdBuffer;
    cmdBuffer.insert(cmdBuffer.end(), parameters.begin(), parameters.end());

    switch (mode)
    {
    case CM_PLAIN: { cmdBuffer.insert(cmdBuffer.end(), data.begin(), data.end());
    }
    break;

    case CM_MAC:
    {
        ByteVector mac = crypto->generateMAC(cmd, data);
        cmdBuffer.insert(cmdBuffer.end(), data.begin(), data.end());
        cmdBuffer.insert(cmdBuffer.end(), mac.begin(), mac.end());
    }
    break;

    case CM_ENCRYPT:
    {
        auto edata = crypto->desfireEncrypt(data);
        cmdBuffer.insert(cmdBuffer.end(), edata.begin(), edata.end());
    }
    break;

    default: THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Unknown crypto mode.");
    }

    unsigned char err;
    size_t pos = 0;
    do
    {
        // Send the data little by little
        size_t pkSize = ((cmdBuffer.size() - pos) >= (DESFIRE_CLEAR_DATA_LENGTH_CHUNK))
                            ? (DESFIRE_CLEAR_DATA_LENGTH_CHUNK)
                            : (cmdBuffer.size() - pos);

        auto command =
            ByteVector(cmdBuffer.begin() + pos, cmdBuffer.begin() + pos + pkSize);

        if (command.size() == 0)
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                     "Data requested but no more data to send.");

        auto result = transmit(pos == 0 ? cmd : DF_INS_ADDITIONAL_FRAME, command);

        err = result.getSW2();
        pos += pkSize;
    } while (err == DF_INS_ADDITIONAL_FRAME);

    if (err != 0x00)
    {
        char msgtmp[64];
        sprintf(msgtmp, "Unknown error: %x", err);
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, msgtmp);
    }
}

void DESFireISO7816Commands::changeFileSettings(unsigned char fileno,
                                                EncryptionMode comSettings,
                                                const DESFireAccessRights &accessRights,
                                                bool plain)
{
    ByteVector command;
    short ar = AccessRightsInMemory(accessRights);

    command.push_back(static_cast<unsigned char>(comSettings));
    BufferHelper::setUShort(command, ar);

    if (!plain)
    {
        ByteVector param;
        param.push_back(DF_INS_CHANGE_FILE_SETTINGS);
        param.push_back(static_cast<unsigned char>(fileno));
        command = getDESFireChip()->getCrypto()->desfireEncrypt(command, param);
    }
    unsigned char fc = static_cast<unsigned char>(fileno);
    command.insert(command.begin(), &fc, &fc + 1);

    transmit(DF_INS_CHANGE_FILE_SETTINGS, command);
}

void DESFireISO7816Commands::createStdDataFile(unsigned char fileno,
                                               EncryptionMode comSettings,
                                               const DESFireAccessRights &accessRights,
                                               unsigned int fileSize)
{
    ByteVector command;
    short ar = AccessRightsInMemory(accessRights);

    command.push_back(fileno);
    command.push_back(static_cast<unsigned char>(comSettings));
    BufferHelper::setUShort(command, ar);
    command.push_back(static_cast<unsigned char>(fileSize & 0xff));
    command.push_back(
        static_cast<unsigned char>(static_cast<unsigned short>(fileSize & 0xff00) >> 8));
    command.push_back(
        static_cast<unsigned char>(static_cast<unsigned int>(fileSize & 0xff0000) >> 16));

    transmit(DF_INS_CREATE_STD_DATA_FILE, command);
}

void DESFireISO7816Commands::createBackupFile(unsigned char fileno,
                                              EncryptionMode comSettings,
                                              const DESFireAccessRights &accessRights,
                                              unsigned int fileSize)
{
    ByteVector command;
    short ar = AccessRightsInMemory(accessRights);

    command.push_back(fileno);
    command.push_back(static_cast<unsigned char>(comSettings));
    BufferHelper::setUShort(command, ar);
    command.push_back(static_cast<unsigned char>(fileSize & 0xff));
    command.push_back(
        static_cast<unsigned char>(static_cast<unsigned short>(fileSize & 0xff00) >> 8));
    command.push_back(
        static_cast<unsigned char>(static_cast<unsigned int>(fileSize & 0xff0000) >> 16));

    transmit(DF_INS_CREATE_BACKUP_DATA_FILE, command);
}

void DESFireISO7816Commands::createValueFile(unsigned char fileno,
                                             EncryptionMode comSettings,
                                             const DESFireAccessRights &accessRights,
                                             int32_t lowerLimit,
                                             int32_t upperLimit, int32_t value,
                                             bool limitedCreditEnabled)
{
    ByteVector command;
    short ar = AccessRightsInMemory(accessRights);

    command.push_back(fileno);
    command.push_back(static_cast<unsigned char>(comSettings));
    BufferHelper::setUShort(command, ar);
    BufferHelper::setInt32(command, static_cast<int>(lowerLimit));
    BufferHelper::setInt32(command, static_cast<int>(upperLimit));
    BufferHelper::setInt32(command, value);
    command.push_back(limitedCreditEnabled ? 0x01 : 0x00);

    transmit(DF_INS_CREATE_VALUE_FILE, command);
}

void DESFireISO7816Commands::createLinearRecordFile(
    unsigned char fileno, EncryptionMode comSettings,
    const DESFireAccessRights &accessRights, unsigned int fileSize,
    unsigned int maxNumberOfRecords)
{
    ByteVector command;
    short ar = AccessRightsInMemory(accessRights);

    command.push_back(fileno);
    command.push_back(static_cast<unsigned char>(comSettings));
    BufferHelper::setUShort(command, ar);
    command.push_back(static_cast<unsigned char>(fileSize & 0xff));
    command.push_back(
        static_cast<unsigned char>(static_cast<unsigned short>(fileSize & 0xff00) >> 8));
    command.push_back(
        static_cast<unsigned char>(static_cast<unsigned int>(fileSize & 0xff0000) >> 16));
    command.push_back(static_cast<unsigned char>(maxNumberOfRecords & 0xff));
    command.push_back(static_cast<unsigned char>(
        static_cast<unsigned short>(maxNumberOfRecords & 0xff00) >> 8));
    command.push_back(static_cast<unsigned char>(
        static_cast<unsigned int>(maxNumberOfRecords & 0xff0000) >> 16));

    transmit(DF_INS_CREATE_LINEAR_RECORD_FILE, command);
}

void DESFireISO7816Commands::createCyclicRecordFile(
    unsigned char fileno, EncryptionMode comSettings,
    const DESFireAccessRights &accessRights, unsigned int fileSize,
    unsigned int maxNumberOfRecords)
{
    ByteVector command;
    short ar = AccessRightsInMemory(accessRights);

    command.push_back(static_cast<unsigned char>(fileno));
    command.push_back(static_cast<unsigned char>(comSettings));
    BufferHelper::setUShort(command, ar);
    command.push_back(static_cast<unsigned char>(fileSize & 0xff));
    command.push_back(
        static_cast<unsigned char>(static_cast<unsigned short>(fileSize & 0xff00) >> 8));
    command.push_back(
        static_cast<unsigned char>(static_cast<unsigned int>(fileSize & 0xff0000) >> 16));
    command.push_back(static_cast<unsigned char>(maxNumberOfRecords & 0xff));
    command.push_back(static_cast<unsigned char>(
        static_cast<unsigned short>(maxNumberOfRecords & 0xff00) >> 8));
    command.push_back(static_cast<unsigned char>(
        static_cast<unsigned int>(maxNumberOfRecords & 0xff0000) >> 16));

    transmit(DF_INS_CREATE_CYCLIC_RECORD_FILE, command);
}

void DESFireISO7816Commands::deleteFile(unsigned char fileno)
{
    ByteVector command;

    command.push_back(fileno);

    transmit(DF_INS_DELETE_FILE, command);
}

ByteVector DESFireISO7816Commands::readData(unsigned char fileno, unsigned int offset,
                                            unsigned int length, EncryptionMode mode)
{
    ByteVector command, ret;

    command.push_back(fileno);

    // Currently we have some problems to read more than 253 bytes with an Omnikey Reader.
    // So the read command is separated to some commands, 8 bytes aligned.

    for (size_t i = 0; i < length; i += 248)
    {
        size_t trunloffset = offset + i;
        size_t trunklength = ((length - i) > 248) ? 248 : (length - i);
        command.push_back(static_cast<unsigned char>(trunloffset & 0xff));
        command.push_back(static_cast<unsigned char>(
            static_cast<unsigned short>(trunloffset & 0xff00) >> 8));
        command.push_back(static_cast<unsigned char>(
            static_cast<unsigned int>(trunloffset & 0xff0000) >> 16));
        command.push_back(static_cast<unsigned char>(trunklength & 0xff));
        command.push_back(static_cast<unsigned char>(
            static_cast<unsigned short>(trunklength & 0xff00) >> 8));
        command.push_back(static_cast<unsigned char>(
            static_cast<unsigned int>(trunklength & 0xff0000) >> 16));

        auto result = transmit(DF_INS_READ_DATA, command);
        result = ISO7816Response(handleReadData(result.getSW2(), result.getData(),
                                static_cast<unsigned int>(trunklength), mode));
        ret.insert(ret.end(), result.getData().begin(), result.getData().end());
    }

    return ret;
}

void DESFireISO7816Commands::writeData(unsigned char fileno, unsigned int offset,
                                       const ByteVector &data, EncryptionMode mode)
{
    ByteVector parameters(7);
    parameters[0] = static_cast<unsigned char>(fileno);
    parameters[1] = static_cast<unsigned char>(offset & 0xff);
    parameters[2] =
        static_cast<unsigned char>(static_cast<unsigned short>(offset & 0xff00) >> 8);
    parameters[3] =
        static_cast<unsigned char>(static_cast<unsigned int>(offset & 0xff0000) >> 16);
    parameters[4] = static_cast<unsigned char>(data.size() & 0xff);
    parameters[5] = static_cast<unsigned char>(
        static_cast<unsigned short>(data.size() & 0xff00) >> 8);
    parameters[6] = static_cast<unsigned char>(
        static_cast<unsigned int>(data.size() & 0xff0000) >> 16);

    handleWriteData(DF_INS_WRITE_DATA, parameters, data, mode);
}

int32_t DESFireISO7816Commands::getValue(unsigned char fileno, EncryptionMode mode)
{
    ByteVector command;
    command.push_back(fileno);

    auto result = transmit(DF_INS_GET_VALUE, command);
    auto data = handleReadData(result.getSW2(), result.getData(), 4, mode);

    if (data.size() >= 4)
    {
        size_t offset = 0;
        return BufferHelper::getInt32(data, offset);
    }
    THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "DESFireISO7816Commands getValue did not return enough data");
}

void DESFireISO7816Commands::credit(unsigned char fileno, unsigned int value,
                                    EncryptionMode mode)
{
    ByteVector parameters(1);
    parameters[0]      = fileno;
    uint32_t dataValue = static_cast<uint32_t>(value);
    ByteVector data(sizeof(dataValue));
    memcpy(&data[0], &dataValue, sizeof(dataValue));
    handleWriteData(DF_INS_CREDIT, parameters, data, mode);
}

void DESFireISO7816Commands::debit(unsigned char fileno, unsigned int value,
                                   EncryptionMode mode)
{
    ByteVector parameters(1);
    parameters[0]      = fileno;
    uint32_t dataValue = static_cast<uint32_t>(value);
    ByteVector data(sizeof(dataValue));
    memcpy(&data[0], &dataValue, sizeof(dataValue));
    handleWriteData(DF_INS_DEBIT, parameters, data, mode);
}

void DESFireISO7816Commands::limitedCredit(unsigned char fileno, int32_t value,
                                           EncryptionMode mode)
{
    ByteVector parameters(1);
    parameters[0]      = static_cast<unsigned char>(fileno);
    ByteVector data(sizeof(value));
    memcpy(&data[0], &value, sizeof(value));
    handleWriteData(DF_INS_LIMITED_CREDIT, parameters, data, mode);
}

void DESFireISO7816Commands::writeRecord(unsigned char fileno, unsigned int offset,
                                         const ByteVector &data, EncryptionMode mode)
{
    ByteVector parameters(7);
    parameters[0] = fileno;
    parameters[1] = static_cast<unsigned char>(offset & 0xff);
    parameters[2] =
        static_cast<unsigned char>(static_cast<unsigned short>(offset & 0xff00) >> 8);
    parameters[3] =
        static_cast<unsigned char>(static_cast<unsigned int>(offset & 0xff0000) >> 16);
    parameters[4] = static_cast<unsigned char>(data.size() & 0xff);
    parameters[5] = static_cast<unsigned char>(
        static_cast<unsigned short>(data.size() & 0xff00) >> 8);
    parameters[6] = static_cast<unsigned char>(
        static_cast<unsigned int>(data.size() & 0xff0000) >> 16);

    handleWriteData(DF_INS_WRITE_RECORD, parameters, data, mode);
}

ByteVector DESFireISO7816Commands::readRecords(unsigned char fileno, unsigned int offset,
                                               unsigned int length, EncryptionMode mode)
{
    ByteVector command, ret;

    command.push_back(fileno);
    command.push_back(static_cast<unsigned char>(offset & 0xff));
    command.push_back(
        static_cast<unsigned char>(static_cast<unsigned short>(offset & 0xff00) >> 8));
    command.push_back(
        static_cast<unsigned char>(static_cast<unsigned int>(offset & 0xff0000) >> 16));
    command.push_back(static_cast<unsigned char>(length & 0xff));
    command.push_back(
        static_cast<unsigned char>(static_cast<unsigned short>(length & 0xff00) >> 8));
    command.push_back(
        static_cast<unsigned char>(static_cast<unsigned int>(length & 0xff0000) >> 16));

    auto result = transmit(DF_INS_READ_RECORDS, command);
    auto data = handleReadData(result.getSW2(), result.getData(), length, mode);
    ret.insert(ret.end(), data.begin(), data.end());

    return ret;
}

void DESFireISO7816Commands::clearRecordFile(unsigned char fileno)
{
    ByteVector command;
    command.push_back(fileno);

    transmit(DF_INS_CLEAR_RECORD_FILE, command);
}

void DESFireISO7816Commands::commitTransaction()
{
    transmit(DF_COMMIT_TRANSACTION);
}

void DESFireISO7816Commands::abortTransaction()
{
    transmit(DF_INS_ABORT_TRANSACTION);
}

void DESFireISO7816Commands::authenticate(unsigned char keyno)
{
    std::shared_ptr<DESFireKey> key = getDESFireChip()->getCrypto()->getKey(0, keyno);
    authenticate(keyno, key);
}

void DESFireISO7816Commands::getKeyFromSAM(std::shared_ptr<DESFireKey> key,
                                           ByteVector diversify) const
{
    auto samKeyStorage = std::dynamic_pointer_cast<SAMKeyStorage>(key->getKeyStorage());

    if (samKeyStorage && samKeyStorage->getDumpKey() &&
        (!getSAMChip() || (getSAMChip()->getCardType() != "SAM_AV2" && getSAMChip()->getCardType() != "SAM_AV3")))
        THROW_EXCEPTION_WITH_LOG(
            LibLogicalAccessException,
            "SAMKeyStorage Dump option can only be used with SAM AV2/AV3.");

    if (key->getKeyDiversification() &&
        !std::dynamic_pointer_cast<NXPAV2KeyDiversification>(
            key->getKeyDiversification()))
        diversify.clear(); // We ignore diversification if wrong type
    key->setData(
        std::dynamic_pointer_cast<SAMAV2Commands<KeyEntryAV2Information, SETAV2>>(
            getSAMChip()->getCommands())
            ->dumpSecretKey(samKeyStorage->getKeySlot(), key->getKeyVersion(),
                            diversify));
    key->setKeyStorage(std::make_shared<ComputerMemoryKeyStorage>());

    // Clear diversification since already done
    if (key->getKeyDiversification() &&
        std::dynamic_pointer_cast<NXPAV2KeyDiversification>(key->getKeyDiversification()))
        key->setKeyDiversification(nullptr);
}

ByteVector DESFireISO7816Commands::sam_authenticate_p1(std::shared_ptr<DESFireKey> key,
                                                       ByteVector rndb,
                                                       ByteVector diversify,
                                                       bool evxauth,
                                                       bool authfirst,
                                                       bool suppresssm) const
{
    unsigned char p1 = 0x00;
    ByteVector data(2);
    std::shared_ptr<SAMKeyStorage> ks = std::dynamic_pointer_cast<SAMKeyStorage>(key->getKeyStorage());
    data[0] = ks->getKeySlot();
    data[1] = key->getKeyVersion();
    if (evxauth)
    {
        data.push_back(0x00); // EV2 auth. 0x01 LRP auth but this is not supported on LLA atm.
    }
    data.insert(data.end(), rndb.begin(), rndb.end());

    if (std::dynamic_pointer_cast<NXPKeyDiversification>(key->getKeyDiversification()))
    {
        p1 |= 0x01;
        if (std::dynamic_pointer_cast<NXPAV2KeyDiversification>(
                key->getKeyDiversification()))
            p1 |= 0x10;
        data.insert(data.end(), diversify.begin(), diversify.end());
    }

    if (!ks->getIsAbsolute())
    {
        p1 |= 0x02; // Use Key Slot as a relative DESFire Key Number instead
    }

    if (evxauth)
    {
        p1 |= 0x80;
        if (!authfirst)
        {
            p1 = 0x40;
        }
    }
    if (suppresssm)
    {
        p1 |= 0x20; // Suppress Secure Messaging (for Originality Check)
    }

    ByteVector cmd_vector = {
        0x80, 0x0a, p1, 0x00, static_cast<unsigned char>(data.size()), 0x00};
    cmd_vector.insert(cmd_vector.end() - 1, data.begin(), data.end());

    int trytoreconnect = 0;
    ByteVector apduresult;
    do
    {
        try
        {
            if (getSAMChip()->getCardType() == "SAM_AV1")
                apduresult = std::dynamic_pointer_cast<
                                 SAMCommands<KeyEntryAV1Information, SETAV1>>(
                                 getSAMChip()->getCommands())
                                 ->transmit(cmd_vector);
            else if (getSAMChip()->getCardType() == "SAM_AV2" || getSAMChip()->getCardType() == "SAM_AV3")
                apduresult = std::dynamic_pointer_cast<
                                 SAMCommands<KeyEntryAV2Information, SETAV2>>(
                                 getSAMChip()->getCommands())
                                 ->transmit(cmd_vector, true, false);
            break;
        }
        catch (CardException &ex)
        {
            if (trytoreconnect > 5 || ex.error_code() != CardException::WRONG_P1_P2 ||
                !std::dynamic_pointer_cast<NXPKeyDiversification>(
                    key->getKeyDiversification()))
                std::rethrow_exception(std::current_exception());

            // SAM av2 often fail even if parameters are correct for during
            // diversification av2
            LOG(LogLevel::WARNINGS) << "try to auth with SAM P1: " << trytoreconnect;
            getSAMChip()
                ->getCommands()
                ->getReaderCardAdapter()
                ->getDataTransport()
                ->getReaderUnit()
                ->reconnect(0);
        }
        ++trytoreconnect;
    } while (true);

    if (apduresult.size() <= 2)
        THROW_EXCEPTION_WITH_LOG(CardException, "sam authenticate DES P1 failed.");

    return ByteVector(apduresult.begin(), apduresult.end() - 2);
}

void DESFireISO7816Commands::sam_authenticate_p2(unsigned char keyno,
                                                 ByteVector rndap) const
{
    std::shared_ptr<DESFireCrypto> crypto = getDESFireChip()->getCrypto();
    ByteVector cmd_vector = {0x80, 0x0a, 0x00, 0x00, static_cast<uint8_t>(rndap.size())};
    cmd_vector.insert(cmd_vector.end(), rndap.begin(), rndap.end());
    ByteVector apduresult;
    if (getSAMChip()->getCardType() == "SAM_AV1")
        apduresult =
            std::dynamic_pointer_cast<SAMCommands<KeyEntryAV1Information, SETAV1>>(
                getSAMChip()->getCommands())
                ->transmit(cmd_vector);
    else if (getSAMChip()->getCardType() == "SAM_AV2" || getSAMChip()->getCardType() == "SAM_AV3")
        apduresult =
            std::dynamic_pointer_cast<SAMCommands<KeyEntryAV2Information, SETAV2>>(
                getSAMChip()->getCommands())
                ->transmit(cmd_vector, true, false);
    if (apduresult.size() != 2 || apduresult[0] != 0x90 || apduresult[1] != 0x00)
        THROW_EXCEPTION_WITH_LOG(CardException, "sam authenticate DES P2 failed.");

    if (getSAMChip()->getCardType() == "SAM_AV1")
    {
        crypto->d_sessionKey =
            std::dynamic_pointer_cast<SAMCommands<KeyEntryAV1Information, SETAV1>>(
                getSAMChip()->getCommands())
                ->dumpSessionKey();
    }
    else if (getSAMChip()->getCardType() == "SAM_AV2" || getSAMChip()->getCardType() == "SAM_AV3")
    {
        auto data = std::dynamic_pointer_cast<SAMCommands<KeyEntryAV2Information, SETAV2>>(
                        getSAMChip()->getCommands())->dumpSessionKey();
        crypto->d_sessionKey = data;
    }
    crypto->d_currentKeyNo = keyno;
}

void DESFireISO7816Commands::authenticate(unsigned char keyno,
                                          std::shared_ptr<DESFireKey> currentKey)
{
    ByteVector command;

    std::shared_ptr<DESFireCrypto> crypto = getDESFireChip()->getCrypto();
    if (!currentKey)
    {
        currentKey = crypto->getDefaultKey(DF_KEY_DES);
    }
    std::shared_ptr<DESFireKey> key = std::make_shared<DESFireKey>(*currentKey);

    auto diversify = getKeyInformations(key, keyno);

    auto samKeyStorage = std::dynamic_pointer_cast<SAMKeyStorage>(key->getKeyStorage());
    if (samKeyStorage && !getSAMChip())
        THROW_EXCEPTION_WITH_LOG(
            LibLogicalAccessException,
            "SAMKeyStorage set on the key but no SAM reader has been set.");

    crypto->setKey(crypto->d_currentAid, 0, keyno, key);

    command.push_back(keyno);

    auto result = DESFireISO7816Commands::transmit(DF_INS_AUTHENTICATE, command);
    if (result.getSW2() == DF_INS_ADDITIONAL_FRAME && result.getData().size() >= 8)
    {
        ByteVector rndAB, apduresult;

        if (samKeyStorage)
            rndAB = sam_authenticate_p1(key, result.getData(), diversify);
        else
            rndAB = crypto->authenticate_PICC1(keyno, diversify, result.getData());
        result    = DESFireISO7816Commands::transmit(DF_INS_ADDITIONAL_FRAME, rndAB);
        if (result.getData().size() >= 8)
        {
            if (samKeyStorage)
                sam_authenticate_p2(keyno, result.getData());
            else
                crypto->authenticate_PICC2(keyno, result.getData());
        }
        else
            THROW_EXCEPTION_WITH_LOG(CardException, "DESFire authentication P2 failed: wrong length.");
    }
    else
        THROW_EXCEPTION_WITH_LOG(CardException, "DESFire authentication P1 failed.");
}

ISO7816Response DESFireISO7816Commands::transmit(unsigned char cmd, unsigned char lc)
{
    return transmit(cmd, ByteVector(), lc, true);
}

ISO7816Response DESFireISO7816Commands::transmit(unsigned char cmd,
                                                 const ByteVector &data, unsigned char lc,
                                                 bool forceLc)
{
    if (data.size())
    {
        return getISO7816ReaderCardAdapter()->sendAPDUCommand(
            DF_CLA_ISO_WRAP, cmd, 0x00, 0x00, static_cast<unsigned char>(data.size()),
            data, 0x00);
    }
    if (forceLc)
    {
        return getISO7816ReaderCardAdapter()->sendAPDUCommand(DF_CLA_ISO_WRAP, cmd, 0x00,
                                                              0x00, lc, 0x00);
    }

    return getISO7816ReaderCardAdapter()->sendAPDUCommand(DF_CLA_ISO_WRAP, cmd, 0x00,
                                                          0x00, 0x00);
}

void DESFireISO7816Commands::setChip(std::shared_ptr<Chip> chip)
{
    DESFireCommands::setChip(chip);
    getDESFireChip()->getCrypto()->setCryptoContext(chip->getChipIdentifier());
}
}
