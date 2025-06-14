#include <logicalaccess/plugins/readers/iso7816/commands/desfireev1iso7816commands.hpp>
#include <logicalaccess/plugins/readers/iso7816/commands/samav2iso7816commands.hpp>
#include <cassert>
#include <logicalaccess/plugins/cards/desfire/desfireev2crypto.hpp>
#include <logicalaccess/plugins/readers/iso7816/commands/desfireev2iso7816commands.hpp>
#include <logicalaccess/plugins/readers/iso7816/commands/desfireiso7816resultchecker.hpp>
#include <logicalaccess/cards/samkeystorage.hpp>
#include <logicalaccess/plugins/crypto/lla_random.hpp>
#include <logicalaccess/plugins/crypto/cmac.hpp>
#include <logicalaccess/plugins/crypto/signature_helper.hpp>
#include <logicalaccess/plugins/cards/desfire/desfirechip.hpp>
#include <algorithm>
#include <logicalaccess/plugins/crypto/aes_helper.hpp>
#include <logicalaccess/plugins/crypto/des_helper.hpp>
#include <atomic>
#include <logicalaccess/utils.hpp>
#include <logicalaccess/plugins/llacommon/settings.hpp>
#include <logicalaccess/plugins/readers/iso7816/guardswap.hpp>

using namespace logicalaccess;

DESFireEV2ISO7816Commands::DESFireEV2ISO7816Commands()
    : DESFireEV1ISO7816Commands(CMD_DESFIREEV2ISO7816)
{
}

DESFireEV2ISO7816Commands::DESFireEV2ISO7816Commands(const std::string &cmd_type)
    : DESFireEV1ISO7816Commands(cmd_type)
{
}

void DESFireEV2ISO7816Commands::changeKeyEV2(uint8_t keysetno, uint8_t keyno,
                                             std::shared_ptr<DESFireKey> newkey)
{
    std::shared_ptr<DESFireCrypto> crypto = getDESFireChip()->getCrypto();
    std::shared_ptr<DESFireKey> oldkey =
        std::dynamic_pointer_cast<DESFireKey>(crypto->getKey(keysetno, keyno));

    bool samChangeKey = checkChangeKeySAMKeyStorage(keyno, oldkey, newkey);

    if (!samChangeKey && keysetno == 0) // it is the same as ChangeKey
        return changeKey(keyno, newkey);

    std::shared_ptr<DESFireKey> key       = std::make_shared<DESFireKey>(*newkey);

    auto oldKeyDiversify = getKeyInformations(crypto->getKey(0, keyno), keyno);
    auto newKeyDiversify = getKeyInformations(key, keyno);

    unsigned char keynobyte = keyno;
    if (keyno == 0 && crypto->d_currentAid == 0)
    {
        keynobyte |= key->getKeyType();
    }

    ByteVector cryptogram;
    if (samChangeKey)
    {
        cryptogram = getChangeKeySAMCryptogram(keyno, key, true, keysetno);
    }
    else
    {
        // When changing key from another keyset, always force new + old key crypto
        cryptogram = crypto->changeKey_PICC(keynobyte, oldKeyDiversify, key, newKeyDiversify, keysetno);
    }

    ByteVector data;
    data.push_back(keysetno);
    data.push_back(keynobyte);
    data.insert(data.end(), cryptogram.begin(), cryptogram.end());

    ByteVector dd = transmit_plain(DFEV2_INS_CHANGEKEYEV2, data).getData();
    if (dd.size() > 0)
    {
        if (!crypto->verifyMAC(true, dd))
        {
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                     "MAC verification failed.");
        }
    }

    crypto->setKey(crypto->d_currentAid, keysetno, keyno, newkey);
}

void DESFireEV2ISO7816Commands::authenticate(unsigned char keyno,
                                             std::shared_ptr<DESFireKey> key)
{
    EXCEPTION_ASSERT_WITH_LOG(key != nullptr, LibLogicalAccessException,
                              "Key shall not be null");

    if (key->getKeyType() != DF_KEY_AES)
    {
        DESFireEV1ISO7816Commands::authenticate(keyno, key);
        return;
    }

    // ChangeKey PICC AES -> DES do not work with authenticateEV2First so we use
    // authenticateAES...
    // ... but we don't care too much about this use case, and user can still
    // call authenticateAES on their own.

    if (key->getKeyStorage()->getType() == KST_SAM &&
        !std::dynamic_pointer_cast<SAMKeyStorage>(key->getKeyStorage())->getDumpKey())
    {
            auto crypto = std::dynamic_pointer_cast<DESFireEV2Crypto>(getDESFireChip()->getCrypto());
            crypto->setKey(crypto->d_currentAid, 0, keyno, key);
            sam_authenticateEV2First(keyno, key);
    }
    else
    {
            authenticateEV2First(keyno, key);
    }
    onAuthenticated();
}

void DESFireEV2ISO7816Commands::authenticateEV2First(
    uint8_t keyno, std::shared_ptr<DESFireKey> currentKey)
{
    auto crypto =
        std::dynamic_pointer_cast<DESFireEV2Crypto>(getDESFireChip()->getCrypto());

    if (!currentKey)
        currentKey = crypto->getDefaultKey(DF_KEY_AES);

    EXCEPTION_ASSERT_WITH_LOG(
        currentKey && currentKey->getKeyType() == DESFireKeyType::DF_KEY_AES,
        LibLogicalAccessException, "Invalid key type: expected DF_KEY_AES");
    ByteVector cmd = {keyno, 0}; // LenCap

    auto response = transmit_plain(DFEV2_INS_AUTHENTICATE_EV2_FIRST, cmd);
    EXCEPTION_ASSERT_WITH_LOG(response.getData().size() > 0, LibLogicalAccessException,
                              "Response is too short");

    ByteVector keydiv;
    std::shared_ptr<DESFireKey> key = std::make_shared<DESFireKey>(*currentKey);
    ByteVector diversify            = getKeyInformations(key, keyno);

    crypto->getKey(key, diversify, keydiv);

    auto key_data       = keydiv;
    auto rnda           = RandomHelper::bytes(16);
    auto rndb           = AESHelper::AESDecrypt(response.getData(), key_data, {});
    auto challenge_resp = DESFireEV2Crypto::get_challenge_response(rndb, rnda);
    challenge_resp      = AESHelper::AESEncrypt(challenge_resp, key_data, {});
    response            = transmit_plain(DF_INS_ADDITIONAL_FRAME, challenge_resp);
    EXCEPTION_ASSERT_WITH_LOG(response.getData().size() >= 32, LibLogicalAccessException,
                              "Response is too short");
    auto decdata = AESHelper::AESDecrypt(response.getData(), key_data, {});

    // throw on error.
    auto ti = DESFireEV2Crypto::auth_ev2_part2(decdata, rnda);

    crypto->setTI(ti);
    crypto->setCmdCtr(0);
    crypto->generateSessionKey(key_data, rnda, rndb);

    getDESFireChip()->getCrypto()->d_auth_method  = CryptoMethod::CM_EV2;
    getDESFireChip()->getCrypto()->d_currentKeyNo = keyno;

    crypto->setKey(crypto->d_currentAid, 0, keyno, key);
}

void DESFireEV2ISO7816Commands::sam_authenticateEV2First(uint8_t keyno,
                                                         std::shared_ptr<DESFireKey> key)
{
    // Check needed info
    EXCEPTION_ASSERT_WITH_LOG(key->getKeyType() == DESFireKeyType::DF_KEY_AES,
                              LibLogicalAccessException,
                              "Invalid key type: expected DF_KEY_AES");
    EXCEPTION_ASSERT_WITH_LOG(getSAMChip() != nullptr && (getSAMChip()->getCardType() == "SAM_AV2" || getSAMChip()->getCardType() == "SAM_AV3"),
                              LibLogicalAccessException,
                              "SAM AV2/AV3 is needed for DESFireEV2");

    auto crypto =
        std::dynamic_pointer_cast<DESFireEV2Crypto>(getDESFireChip()->getCrypto());

    ByteVector diversify = getKeyInformations(key, keyno);

    auto keystorage = std::dynamic_pointer_cast<SAMKeyStorage>(key->getKeyStorage());
    if (!keystorage)
        THROW_EXCEPTION_WITH_LOG(
            LibLogicalAccessException,
            "DESFireKey need a SAMKeyStorage to proceed a SAM Authenticate.");

            
    if (getSAMChip()->getCardType() == "SAM_AV2")
    {
        authenticateAES(keyno); // this should be removed, we keep it as a safekeeper for now to easer auth error analyze.
        const auto rndPcd = RandomHelper::bytes(16);

        // activate offline crypto with SAM
        auto samAV2Cmd =
            std::dynamic_pointer_cast<SAMAV2ISO7816Commands>(getSAMChip()->getCommands());
        samAV2Cmd->activateOfflineKey(keystorage->getKeySlot(), key->getKeyVersion(),
                                    diversify);

        // Start DESFireEV2 auth
        ByteVector cmd = {keyno, 0}; // LenCap
        auto response  = transmit_plain(DFEV2_INS_AUTHENTICATE_EV2_FIRST, cmd);
        EXCEPTION_ASSERT_WITH_LOG(response.getData().size() > 0, LibLogicalAccessException,
                                "Response is too short");

        auto key_data   = key->getData();

        // Get RNDB
        const auto rndPicc = samAV2Cmd->decipherOfflineData(response.getData());
        EXCEPTION_ASSERT_WITH_LOG(rndPicc.size() >= 16, LibLogicalAccessException,
                                "Response is too short");

        // Get challenge_resp
        auto challenge_resp = DESFireEV2Crypto::get_challenge_response(rndPicc, rndPcd);
        challenge_resp      = samAV2Cmd->encipherOfflineData(challenge_resp);
        EXCEPTION_ASSERT_WITH_LOG(challenge_resp.size() >= 0x20, LibLogicalAccessException,
                                "Response is too short");

        response = transmit_plain(DF_INS_ADDITIONAL_FRAME, challenge_resp);
        EXCEPTION_ASSERT_WITH_LOG(response.getData().size() >= 32, LibLogicalAccessException,
                                "Response is too short");

        auto decdata = samAV2Cmd->decipherOfflineData(response.getData());
        auto ti      = DESFireEV2Crypto::auth_ev2_part2(decdata, rndPcd);

        crypto->setTI(ti);
        crypto->setCmdCtr(0);
        crypto->sam_generateSessionKey(rndPcd, rndPicc, samAV2Cmd);
    }
    else
    {
        ByteVector cmd = {keyno, 0}; // LenCap
        auto response  = transmit_plain(DFEV2_INS_AUTHENTICATE_EV2_FIRST, cmd);
        EXCEPTION_ASSERT_WITH_LOG(response.getData().size() > 0, LibLogicalAccessException,
                                "Response is too short");

        bool suppresssm = crypto->d_currentAid == 0 && keyno > 0 && !keystorage->getIsAbsolute(); // should only match Originality Keys
        auto command = sam_authenticate_p1(key, response.getData(), diversify, true, true, suppresssm);
        auto result = DESFireISO7816Commands::transmit(DF_INS_ADDITIONAL_FRAME, command);
        if (result.getData().size() >= 8)
        {
            sam_authenticate_p2(keyno, result.getData(), true);
        }
        else
        {
            THROW_EXCEPTION_WITH_LOG(CardException, "DESFire authentication P2 failed: wrong length.");
        }
    }

    crypto->d_auth_method  = CryptoMethod::CM_EV2;
    crypto->d_mac_size   = 8;
    crypto->d_lastIV.clear();
    crypto->d_cipher.reset(new openssl::AESCipher());
    crypto->d_lastIV.resize(crypto->d_cipher->getBlockSize(), 0x00);
    crypto->d_currentKeyNo = keyno;
}

void DESFireEV2ISO7816Commands::sam_authenticate_p2(unsigned char keyno,
    ByteVector rndap, bool authev2) const
{
    DESFireISO7816Commands::sam_authenticate_p2(keyno, rndap);
    if (authev2)
    {
        auto crypto = std::dynamic_pointer_cast<DESFireEV2Crypto>(getDESFireChip()->getCrypto());
        auto data = crypto->d_sessionKey;
        EXCEPTION_ASSERT_WITH_LOG(data.size() >= 38, LibLogicalAccessException,
                                "Response is too short");

        crypto->d_macSessionKey = ByteVector(data.begin() + 16, data.begin() + 32);
        crypto->setTI(ByteVector(data.begin() + 32, data.begin() + 36));
        crypto->setCmdCtr(static_cast<uint16_t>(data.at(36) << 8 | data.at(37)));
        crypto->d_sessionKey = ByteVector(data.begin(), data.begin() + 16);

        LOG(LogLevel::DEBUGS) << "DESFireEV2 sam_authenticate_p2 done.";
    }
}

ISO7816Response DESFireEV2ISO7816Commands::transmit(unsigned char cmd,
                                                    const ByteVector &buf,
                                                    unsigned char lc, bool forceLc)
{
    auto crypto =
        std::dynamic_pointer_cast<DESFireEV2Crypto>(getDESFireChip()->getCrypto());
    if (crypto->d_auth_method != CryptoMethod::CM_EV2)
        return DESFireEV1ISO7816Commands::transmit(cmd, buf, lc, forceLc);

    auto command = crypto->generateMAC(cmd, buf);
    command.insert(command.begin(), buf.begin(), buf.end());
    auto result = transmit_plain(cmd, command);

    crypto->verifyMAC(true, result.getData());

    return ISO7816Response(
        ByteVector(result.getData().begin(), result.getData().end() - 8), result.getSW1(),
        result.getSW2());
}

ISO7816Response DESFireEV2ISO7816Commands::transmit_nomacv(unsigned char cmd,
                                                           const ByteVector &buf,
                                                           unsigned char lc, bool forceLc)
{
    auto crypto =
        std::dynamic_pointer_cast<DESFireEV2Crypto>(getDESFireChip()->getCrypto());
    if (crypto->d_auth_method != CryptoMethod::CM_EV2)
        return DESFireEV1ISO7816Commands::transmit_nomacv(cmd, buf, lc, forceLc);

    auto command = crypto->generateMAC(cmd, buf);
    command.insert(command.begin(), buf.begin(), buf.end());
    auto result = transmit_plain(cmd, command);

    return result;
}

ISO7816Response DESFireEV2ISO7816Commands::handleReadCmd(unsigned char cmd,
                                                         const ByteVector &data,
                                                         EncryptionMode mode)
{
    auto crypto =
        std::dynamic_pointer_cast<DESFireEV2Crypto>(getDESFireChip()->getCrypto());
    if (crypto->d_auth_method != CryptoMethod::CM_EV2 || mode != CM_PLAIN)
        return DESFireEV1ISO7816Commands::handleReadCmd(cmd, data, mode);

    auto result = transmit_plain(cmd, data); // EV2 has a real plain mode
    crypto->setCmdCtr(crypto->getCmdCtr() + 1);
    return result;
}

ByteVector DESFireEV2ISO7816Commands::handleReadData(unsigned char err,
                                                     const ByteVector &firstMsg,
                                                     unsigned int length,
                                                     EncryptionMode mode)
{
    auto crypto =
        std::dynamic_pointer_cast<DESFireEV2Crypto>(getDESFireChip()->getCrypto());
    if (crypto->d_auth_method != CryptoMethod::CM_EV2 || mode != CM_PLAIN)
        return DESFireEV1ISO7816Commands::handleReadData(err, firstMsg, length, mode);

    // EV2 has a real plain mode
    ByteVector ret = firstMsg;
    while (err == DF_INS_ADDITIONAL_FRAME)
    {
        auto data = transmit_plain(DF_INS_ADDITIONAL_FRAME);
        err       = data.getSW2();
        ret.insert(ret.end(), data.getData().begin(), data.getData().end());
    }

    return ret;
}

void DESFireEV2ISO7816Commands::handleWriteData(unsigned char cmd,
                                                const ByteVector &parameters,
                                                const ByteVector &data,
                                                EncryptionMode mode)
{
    auto crypto =
        std::dynamic_pointer_cast<DESFireEV2Crypto>(getDESFireChip()->getCrypto());
    if (crypto->d_auth_method != CryptoMethod::CM_EV2)
        return DESFireEV1ISO7816Commands::handleWriteData(cmd, parameters, data, mode);

    ByteVector command;
    size_t DESFIRE_EV1_CLEAR_DATA_LENGTH_CHUNK = (DESFIREEV1_CLEAR_DATA_LENGTH_CHUNK - 8);

    crypto->initBuf();

    ByteVector cmdBuffer;
    ISO7816Response result;
    cmdBuffer.insert(cmdBuffer.end(), parameters.begin(), parameters.end());

    // Calcul the full data to send
    switch (mode)
    {
    case CM_PLAIN:
    {
        cmdBuffer.insert(cmdBuffer.end(), data.begin(), data.end());
    }
    break;

    case CM_MAC:
    {
        command.insert(command.end(), parameters.begin(), parameters.end());
        command.insert(command.end(), data.begin(), data.end());
        auto mac = crypto->generateMAC(cmd, command);
        cmdBuffer.insert(cmdBuffer.end(), data.begin(), data.end());
        cmdBuffer.insert(cmdBuffer.end(), mac.begin(), mac.end());
    }
    break;

    case CM_ENCRYPT:
    {
        ByteVector encparameters;
        encparameters.push_back(cmd);
        encparameters.insert(encparameters.end(), parameters.begin(), parameters.end());
        auto edata = crypto->desfireEncrypt(data, encparameters);
        cmdBuffer.insert(cmdBuffer.end(), edata.begin(), edata.end());
    }
    break;

    default:
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "Unknown DESFire crypto mode.");
    }


    size_t pos = 0;
    do
    {
        // Send the data little by little
        size_t pkSize =
            ((cmdBuffer.size() - pos) >= (DESFIRE_EV1_CLEAR_DATA_LENGTH_CHUNK))
                ? (DESFIRE_EV1_CLEAR_DATA_LENGTH_CHUNK)
                : (cmdBuffer.size() - pos);

        command = ByteVector(cmdBuffer.begin() + pos, cmdBuffer.begin() + pos + pkSize);

        if (command.size() == 0)
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                     "Data requested but no more data to send.");

        result = transmit_plain(pos == 0 ? cmd : DF_INS_ADDITIONAL_FRAME, command);

        pos += pkSize;
    } while (result.getSW2() == DF_INS_ADDITIONAL_FRAME);

    if (result.getSW2() != 0x00)
    {
        char msgtmp[64];
        sprintf(msgtmp, "Unknown error: %x", result.getSW2());
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, msgtmp);
    }

    if (result.getData().size() > 0 && mode != CM_PLAIN)
    {
        if (!crypto->verifyMAC(true, result.getData()))
        {
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                     "MAC verification failed.");
        }
    }
    else
    {
        // PLAIN
        crypto->setCmdCtr(crypto->getCmdCtr() + 1);
    }
}

void DESFireEV2ISO7816Commands::changeKey(unsigned char keyno,
                                          std::shared_ptr<DESFireKey> newkey)
{
    DESFireEV1ISO7816Commands::changeKey(keyno, newkey);
}

void DESFireEV2ISO7816Commands::createApplication(
    unsigned int aid, DESFireKeySettings settings, unsigned char maxNbKeys,
    DESFireKeyType cryptoMethod, FidSupport fidSupported, unsigned short isoFID,
    ByteVector isoDFName, unsigned char numberKeySets, unsigned char maxKeySize,
    unsigned char actualkeySetVersion, unsigned char rollkeyno,
    bool specificCapabilityData, bool specificVCKeys)
{
    ByteVector command;
    std::shared_ptr<DESFireCrypto> crypto = getDESFireChip()->getCrypto();

    DESFireLocation::convertUIntToAid(aid, command);
    command.push_back(static_cast<unsigned char>(settings)); // keySett1
    command.push_back(static_cast<unsigned char>((maxNbKeys & 0x0f) | fidSupported |
                                                 cryptoMethod)); // keySett2

    unsigned char keysett3 = (numberKeySets != 0);
    if (keysett3 || specificVCKeys || specificCapabilityData)
    {
        command[4] |= 0x10; // enable KeySett3 on keySett2 flag
        keysett3 |= specificVCKeys ? 0x02 : 0x00;
        keysett3 |= specificCapabilityData ? 0x04 : 0x00;
        command.push_back(static_cast<unsigned char>(keysett3)); // keySett3
    }

    if (numberKeySets != 0)
    {
        command.push_back(static_cast<unsigned char>(actualkeySetVersion)); // AKSVersion
        command.push_back(static_cast<unsigned char>(numberKeySets));       // NoKeySets

        if (maxKeySize == 0)
        {
            maxKeySize = 16; // AES - DES
            if (cryptoMethod == DF_KEY_3K3DES)
                maxKeySize = 24;
        }
        command.push_back(static_cast<unsigned char>(maxKeySize));       // MaxKeySize
        command.push_back(static_cast<unsigned char>(rollkeyno & 0x07)); // AppKeySetSett
    }

    if (isoFID != 0x00)
    {
        command.push_back(static_cast<unsigned char>((isoFID & 0xff00) >> 8));
        command.push_back(static_cast<unsigned char>(isoFID & 0xff));
    }
    if (isoDFName.size() > 0)
    {
        command.insert(command.end(), isoDFName.begin(), isoDFName.end());
    }

    transmit(DF_INS_CREATE_APPLICATION, command);

    crypto->createApplication(aid, numberKeySets + 1, maxNbKeys, cryptoMethod);
}

void DESFireEV2ISO7816Commands::initializeKeySet(uint8_t keySetNo,
                                                DESFireKeyType keySetType)
{
    ByteVector parameters;

    parameters.push_back(keySetNo & 0x07);
    uint8_t keySetTypeValue = 0x00;
    switch (keySetType)
    {
    case DF_KEY_DES: keySetTypeValue = 0x00; break;
    case DF_KEY_3K3DES: keySetTypeValue = 0x01; break;
    case DF_KEY_AES: keySetTypeValue = 0x02; break;
    default: THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Unknown keytype.");
    }

    parameters.push_back(keySetTypeValue);

    auto crypto =
        std::dynamic_pointer_cast<DESFireEV2Crypto>(getDESFireChip()->getCrypto());

    handleWriteData(DFEV2_INS_INITIALIZEKEYSET, parameters, {}, CM_MAC);

    crypto->duplicateCurrentKeySet(keySetNo);
    crypto->setKeySetCryptoType(keySetNo, keySetType);
}

void DESFireEV2ISO7816Commands::rollKeySet(uint8_t keySetNo)
{
    ByteVector parameters;

    parameters.push_back(keySetNo);

    auto crypto =
        std::dynamic_pointer_cast<DESFireEV2Crypto>(getDESFireChip()->getCrypto());

    handleWriteData(DFEV2_INS_ROLLKEYSET, parameters, {}, CM_MAC);

    std::dynamic_pointer_cast<DESFireEV2Crypto>(getDESFireChip()->getCrypto())
        ->duplicateKeySet(0, keySetNo);
}

void DESFireEV2ISO7816Commands::finalizeKeySet(uint8_t keySetNo, uint8_t keySetVersion)
{
    ByteVector parameters;

    parameters.push_back(keySetNo);
    parameters.push_back(keySetVersion);

    handleWriteData(DFEV2_INS_FINALIZEKEYSET, parameters, {}, CM_MAC);
}


void DESFireEV2ISO7816Commands::changeFileSettings(
    unsigned char fileno, EncryptionMode comSettings,
    std::vector<DESFireAccessRights> accessRights)
{
    EXCEPTION_ASSERT_WITH_LOG(accessRights.size() > 0 && accessRights.size() <= 8,
                              LibLogicalAccessException,
                              "Invalid accessrights has been provided.");

    if (accessRights.size() == 1)
        return DESFireEV1ISO7816Commands::changeFileSettings(fileno, comSettings,
                                                             accessRights[0], false);

    std::shared_ptr<DESFireCrypto> crypto = getDESFireChip()->getCrypto();
    ByteVector command;

    unsigned char fileOpt = static_cast<unsigned char>(comSettings);
    fileOpt |= 0x80; // Multi access rights
    command.push_back(fileOpt);

    short ar = AccessRightsInMemory(accessRights[0]);
    BufferHelper::setUShort(command, ar);

    command.push_back(static_cast<unsigned char>(accessRights.size() - 1));
    for (int x = 1; x < static_cast<int>(accessRights.size()); ++x)
    {
        ar = AccessRightsInMemory(accessRights[x]);
        BufferHelper::setUShort(command, ar);
    }
    
    ByteVector param;
    param.push_back(static_cast<unsigned char>(fileno));

    transmit_full(DF_INS_CHANGE_FILE_SETTINGS, command, param).getData();
}

void DESFireEV2ISO7816Commands::createStdDataFile(unsigned char fileno,
                                                  EncryptionMode comSettings,
                                                  const DESFireAccessRights &accessRights,
                                                  unsigned int fileSize,
                                                  unsigned short isoFID,
                                                  bool multiAccessRights)
{
    if (multiAccessRights)
    {
        int tmpComSettings = comSettings;
        tmpComSettings |= 0x80; // Additional access rights
        comSettings = static_cast<EncryptionMode>(tmpComSettings);
    }
    DESFireEV1ISO7816Commands::createStdDataFile(fileno, comSettings, accessRights,
                                                 fileSize, isoFID);
}

void DESFireEV2ISO7816Commands::createBackupFile(unsigned char fileno,
                                                 EncryptionMode comSettings,
                                                 const DESFireAccessRights &accessRights,
                                                 unsigned int fileSize,
                                                 unsigned short isoFID,
                                                 bool multiAccessRights)
{
    if (multiAccessRights)
    {
        int tmpComSettings = comSettings;
        tmpComSettings |= 0x80; // Additional access rights
        comSettings = static_cast<EncryptionMode>(tmpComSettings);
    }
    DESFireEV1ISO7816Commands::createBackupFile(fileno, comSettings, accessRights,
                                                fileSize, isoFID);
}

void DESFireEV2ISO7816Commands::createLinearRecordFile(
    unsigned char fileno, EncryptionMode comSettings,
    const DESFireAccessRights &accessRights, unsigned int fileSize,
    unsigned int maxNumberOfRecords, unsigned short isoFID, bool multiAccessRights)
{
    if (multiAccessRights)
    {
        int tmpComSettings = comSettings;
        tmpComSettings |= 0x80; // Additional access rights
        comSettings = static_cast<EncryptionMode>(tmpComSettings);
    }
    DESFireEV1ISO7816Commands::createLinearRecordFile(
        fileno, comSettings, accessRights, fileSize, maxNumberOfRecords, isoFID);
}

void DESFireEV2ISO7816Commands::createCyclicRecordFile(
    unsigned char fileno, EncryptionMode comSettings,
    const DESFireAccessRights &accessRights, unsigned int fileSize,
    unsigned int maxNumberOfRecords, unsigned short isoFID, bool multiAccessRights)
{
    if (multiAccessRights)
    {
        int tmpComSettings = comSettings;
        tmpComSettings |= 0x80; // Additional access rights
        comSettings = static_cast<EncryptionMode>(tmpComSettings);
    }
    DESFireEV1ISO7816Commands::createCyclicRecordFile(
        fileno, comSettings, accessRights, fileSize, maxNumberOfRecords, isoFID);
}

void DESFireEV2ISO7816Commands::createTransactionMACFile(
    unsigned char fileno, EncryptionMode comSettings,
    const DESFireAccessRights &accessRights, std::shared_ptr<DESFireKey> tmkey)
{
    ByteVector param;
    param.push_back(static_cast<unsigned char>(fileno));      // FileNo
    param.push_back(static_cast<unsigned char>(comSettings)); // FileOption
    short ar = AccessRightsInMemory(accessRights);
    BufferHelper::setUShort(param, ar);                // AccessRights
    param.push_back(static_cast<unsigned char>(0x02)); // TMKeyOption AES only atm

    auto data = tmkey->getData(); // TMKey
    data.push_back(tmkey->getKeyVersion());    // TMKeyVer

    transmit_full(DFEV2_INS_CREATE_TRANSACTION_MAC_FILE, data, param);
}

void DESFireEV2ISO7816Commands::createDelegatedApplication(
    std::pair<ByteVector, ByteVector> damInfo, unsigned int aid, unsigned short DAMSlotNo,
    unsigned char DAMSlotVersion, unsigned short quotatLimit, DESFireKeySettings settings,
    unsigned char maxNbKeys, DESFireKeyType cryptoMethod, FidSupport fidSupported,
    unsigned short isoFID, ByteVector isoDFName, unsigned char numberKeySets,
    unsigned char maxKeySize, unsigned char actualkeySetVersion, unsigned char rollkeyno,
    bool specificCapabilityData, bool specificVCKeys)
{
    std::shared_ptr<DESFireCrypto> crypto = getDESFireChip()->getCrypto();

    auto command = createDelegatedApplicationParam(
        aid, DAMSlotNo, DAMSlotVersion, quotatLimit, settings, maxNbKeys, cryptoMethod,
        fidSupported, isoFID, isoDFName, numberKeySets, maxKeySize, actualkeySetVersion,
        rollkeyno, specificCapabilityData, specificVCKeys);

    auto result = transmit_plain(DFEV2_INS_CREATE_DELEGATED_APPLICATION, command);
    EXCEPTION_ASSERT_WITH_LOG(result.getSW2() == DF_INS_ADDITIONAL_FRAME,
                              LibLogicalAccessException,
                              "CreateDelegatedApplication part 1 failed.");

    command.clear();
    command.insert(command.end(), damInfo.first.begin(), damInfo.first.end());
    command.insert(command.end(), damInfo.second.begin(), damInfo.second.end());

    transmit_plain(DF_INS_ADDITIONAL_FRAME, command);

    crypto->createApplication(aid, numberKeySets + 1, maxNbKeys, cryptoMethod);
}

ByteVector DESFireEV2ISO7816Commands::createDelegatedApplicationParam(
    unsigned int aid, unsigned short DAMSlotNo, unsigned char DAMSlotVersion,
    unsigned short quotatLimit, DESFireKeySettings settings, unsigned char maxNbKeys,
    DESFireKeyType cryptoMethod, FidSupport fidSupported, unsigned short isoFID,
    ByteVector isoDFName, unsigned char numberKeySets, unsigned char maxKeySize,
    unsigned char actualkeySetVersion, unsigned char rollkeyno,
    bool specificCapabilityData, bool specificVCKeys)
{
    ByteVector command;
    DESFireLocation::convertUIntToAid(aid, command);               // AID
    BufferHelper::setUShort(command, DAMSlotNo);                   // DAMSlotNo
    command.push_back(static_cast<unsigned char>(DAMSlotVersion)); // DAMSlotVersion
    BufferHelper::setUShort(command, quotatLimit);                 // quotatLimit
    command.push_back(static_cast<unsigned char>(settings));       // keySett1
    command.push_back(static_cast<unsigned char>((maxNbKeys & 0x0f) | fidSupported |
                                                 cryptoMethod)); // keySett2

    unsigned char keysett3 = (numberKeySets != 0);
    if (keysett3)
    {
        command[9] |= 0x10; // enable KeySett3 on keySett2 flag
        keysett3 |= specificCapabilityData ? 0x02 : 0x00;
        keysett3 |= specificVCKeys ? 0x04 : 0x00;
        command.push_back(static_cast<unsigned char>(keysett3)); // keySett3
    }

    if (numberKeySets != 0)
    {
        command.push_back(static_cast<unsigned char>(actualkeySetVersion)); // AKSVersion
        command.push_back(static_cast<unsigned char>(numberKeySets));       // NoKeySets

        if (maxKeySize == 0)
        {
            maxKeySize = 16; // AES - DES
            if (cryptoMethod == DF_KEY_3K3DES)
                maxKeySize = 24;
        }
        command.push_back(static_cast<unsigned char>(maxKeySize));       // MaxKeySize
        command.push_back(static_cast<unsigned char>(rollkeyno & 0x07)); // AppKeySetSett
    }

    if (isoFID != 0x00)
    {
        command.push_back(static_cast<unsigned char>((isoFID & 0xff00) >> 8));
        command.push_back(static_cast<unsigned char>(isoFID & 0xff));
    }
    if (isoDFName.size() > 0)
    {
        command.insert(command.end(), isoDFName.begin(), isoDFName.end());
    }

    return command;
}

std::pair<ByteVector, ByteVector> DESFireEV2ISO7816Commands::createDAMChallenge(
    std::shared_ptr<DESFireKey> DAMMACKey, std::shared_ptr<DESFireKey> DAMENCKey,
    std::shared_ptr<DESFireKey> DAMDefaultKey, unsigned int aid, unsigned short DAMSlotNo,
    unsigned char DAMSlotVersion, unsigned short quotatLimit, DESFireKeySettings settings,
    unsigned char maxNbKeys, DESFireKeyType cryptoMethod, FidSupport fidSupported,
    unsigned short isoFID, ByteVector isoDFName, unsigned char numberKeySets,
    unsigned char maxKeySize, unsigned char actualkeySetVersion, unsigned char rollkeyno,
    bool specificCapabilityData, bool specificVCKeys)
{
    ByteVector command;
    command.push_back(DFEV2_INS_CREATE_DELEGATED_APPLICATION); // CMD
    auto createDelegatedApplicationParamTmp = createDelegatedApplicationParam(
        aid, DAMSlotNo, DAMSlotVersion, quotatLimit, settings, maxNbKeys, cryptoMethod,
        fidSupported, isoFID, isoDFName, numberKeySets, maxKeySize, actualkeySetVersion,
        rollkeyno, specificCapabilityData, specificVCKeys);
    command.insert(command.end(), createDelegatedApplicationParamTmp.begin(),
                   createDelegatedApplicationParamTmp.end());

    // DAM MAC is created using session EV1 & EV2 algo but not with current session key
    // so we create a DESFireEV2Crypto and set the DAM Keys
    auto cryptoTmp = std::make_shared<DESFireEV2Crypto>();
    if (DAMMACKey->getKeyType() == DF_KEY_AES)
    {
        cryptoTmp->d_cipher.reset(new openssl::AESCipher());
        cryptoTmp->d_auth_method = CM_EV2;
    }
    else
    {
        cryptoTmp->d_cipher.reset(new openssl::DESCipher());
        cryptoTmp->d_auth_method = CM_EV1;
        cryptoTmp->d_mac_size    = 8;
    }

    // Generate encK
    ByteVector keybuffer = DAMDefaultKey->getData();
    keybuffer.resize(24);
    keybuffer.push_back(DAMDefaultKey->getKeyVersion());
    auto randomBytes = RandomHelper::bytes(7);
    keybuffer.insert(keybuffer.begin(), randomBytes.begin(), randomBytes.end());
    cryptoTmp->d_sessionKey = DAMENCKey->getData();
    cryptoTmp->d_lastIV.clear();
    cryptoTmp->d_lastIV.resize(cryptoTmp->d_cipher->getBlockSize(), 0x00);
    ByteVector encK = cryptoTmp->desfireEncrypt(keybuffer, {}, false);

    // Add encK to mac buffer
    command.insert(command.end(), encK.begin(), encK.end());
    cryptoTmp->d_sessionKey    = DAMMACKey->getData();
    cryptoTmp->d_macSessionKey = DAMMACKey->getData();
    cryptoTmp->d_lastIV.clear();
    cryptoTmp->d_lastIV.resize(cryptoTmp->d_cipher->getBlockSize(), 0x00);
    auto dammac = cryptoTmp->generateMAC(DFEV2_INS_CREATE_DELEGATED_APPLICATION, command);

    return std::pair<ByteVector, ByteVector>(encK, dammac);
}

ByteVector DESFireEV2ISO7816Commands::getKeyVersion(uint8_t keysetno, uint8_t keyno)
{
    ByteVector command;

    if (keysetno == 0 && keyno == 0) // Ket set versions retrieval
        keysetno = 0x80;

    keyno |= 0x40; // We always add keysetno

    command.push_back(keyno);
    command.push_back(keysetno);

    auto result = transmit(DF_INS_GET_KEY_VERSION, command);
    EXCEPTION_ASSERT_WITH_LOG(result.getData().size() > 0, LibLogicalAccessException,
                              "Invalid response for getKeyVersion.");
    return result.getData();
}

void DESFireEV2ISO7816Commands::setConfiguration(bool formatCardEnabled,
                                                 bool randomIdEnabled,
                                                 bool PCMandatoryEnabled,
                                                 bool AuthVCMandatoryEnabled)
{
    ResultCheckerSwapper<SetConfiguration0ResultChecker> swapper(*getReaderCardAdapter());

    ByteVector command(1);
    command[0] = (formatCardEnabled ? 0x00u : 0x01u) | (randomIdEnabled ? 0x02u : 0x00u) |
                 (PCMandatoryEnabled ? 0x04u : 0x00u) |
                 (AuthVCMandatoryEnabled ? 0x08u : 0x00u);

    ByteVector param;
    param.push_back(0x00); // PICC App key config
    transmit_full(DFEV1_INS_SET_CONFIGURATION, command, param);
}

void DESFireEV2ISO7816Commands::setConfiguration(uint8_t sak1, uint8_t sak2)
{
    ByteVector command;
    command.push_back(sak1);
    command.push_back(sak2);

    ByteVector param;
    param.push_back(0x03); // SAK Configuration
    transmit_full(DFEV1_INS_SET_CONFIGURATION, command, param);
}

void DESFireEV2ISO7816Commands::setConfiguration(bool D40SecureMessagingEnabled,
                                                 bool EV1SecureMessagingEnabled,
                                                 bool EV2ChainedWritingEnabled)
{
    ByteVector command(2);
    command[1] = ((D40SecureMessagingEnabled) ? 0x00 : 0x01) |
                 ((EV1SecureMessagingEnabled) ? 0x00 : 0x02) |
                 ((EV2ChainedWritingEnabled) ? 0x00 : 0x04);

    ByteVector param;
    param.push_back(0x04); // Secure messaging
    transmit_full(DFEV1_INS_SET_CONFIGURATION, command, param);
}

void DESFireEV2ISO7816Commands::setConfigurationPDCap(uint8_t pdcap1_1, uint8_t pdcap2_5,
                                                      uint8_t pdcap2_6)
{
    ByteVector command(10);
    command[0] = pdcap2_6;
    command[1] = pdcap2_5;
    command[6] = pdcap1_1;
    std::reverse(std::begin(command), std::end(command));

    ByteVector param;
    param.push_back(0x05); // PDCap
    transmit_full(DFEV1_INS_SET_CONFIGURATION, command, param);
}

// TODO CALCUL DAMMAC
void DESFireEV2ISO7816Commands::setConfiguration(ByteVector DAMMAC,
                                                 ByteVector ISODFNameOrVCIID)
{
    EXCEPTION_ASSERT_WITH_LOG(
        (DAMMAC.size() == 8 || DAMMAC.size() == 0) && ISODFNameOrVCIID.size() > 0 &&
            ISODFNameOrVCIID.size() <= 0x10,
        LibLogicalAccessException, "Wrong size of DAMMAC or ISODFName");

    ByteVector command;
    command.insert(command.end(), DAMMAC.begin(), DAMMAC.end());
    command.resize(DAMMAC.size() + 0x10);
    std::copy(ISODFNameOrVCIID.rbegin(), ISODFNameOrVCIID.rend(), command.rbegin());
    // command.insert(command.end(), ISODFName.begin(), ISODFName.end());
    command.push_back(static_cast<unsigned char>(ISODFNameOrVCIID.size()));
    // std::reverse(std::begin(command), std::end(command));

    ByteVector param;
    param.push_back(0x06); // DFNames / VCIID
    transmit_full(DFEV1_INS_SET_CONFIGURATION, command, param);
}

void DESFireEV2ISO7816Commands::proximityCheck(std::shared_ptr<DESFireKey> key,
                                               const uint8_t chunk_size)
{
    assert(chunk_size == 1 || chunk_size == 2 || chunk_size == 4 || chunk_size == 8);
    if (!key)
    {
        key = getDESFireChip()->getCrypto()->getKey(0, DFEV2_PROXIMITY_CHECK_KEY_NO);
    }
    EXCEPTION_ASSERT_WITH_LOG(key != nullptr, LibLogicalAccessException,
                              "No ProximityCheck specified nor found");

    // Bytes we compute our MAC over. This is built over multiple commands.
    ByteVector MAC_SOURCE;
    MAC_SOURCE.push_back(DFEV2_INS_VERIFY_PC);

    // PreparePC
    auto resp = transmit_plain(DFEV2_INS_PREPARE_PC);
    EXCEPTION_ASSERT_WITH_LOG(resp.getData().size() >= 3, LibLogicalAccessException,
                              "Not enough byte in response");
    uint8_t option = resp.getData()[0];
    MAC_SOURCE.push_back(option);

    // Time card claim it will take to answer.
    uint16_t published_response_time = resp.getData()[1] << 8u | resp.getData()[2];
    MAC_SOURCE.push_back(resp.getData()[1]);
    MAC_SOURCE.push_back(resp.getData()[2]);

    bool has_pps1_byte = option & 0x01u;
    uint8_t pps1_byte;
    if (has_pps1_byte)
    {
        EXCEPTION_ASSERT_WITH_LOG(resp.getData().size() == 4, LibLogicalAccessException,
                                  "Not enough byte in response");
        pps1_byte = resp.getData()[3];
        MAC_SOURCE.push_back(pps1_byte);
    }

    // Proximity checks

    // Our random
    auto rnd_c = RandomHelper::bytes(8);
    // Card random, populated over multiple calls.
    ByteVector rnd_r;

    uint8_t byte_transferred = 0;
    while (byte_transferred != 8)
    {
        ByteVector cmd;
        cmd.push_back(chunk_size);
        assert(byte_transferred + chunk_size <= rnd_c.size());
        cmd.insert(cmd.end(), rnd_c.begin() + byte_transferred,
                   rnd_c.begin() + byte_transferred + chunk_size);

        {
            // Timing verification hack.

            // Most of the time we will be using a PCSC transport, therefore
            // I think we can only measure the time of whole command/response
            // transaction. Therefore we are doubling the published response.
            // It might prove to not be enough because of all the
            // layers between here and the RFID communication layer.
            ElapsedTimeCounter etc;
            resp              = transmit_plain(DFEV2_INS_PROXIMITY_CHECK, cmd);
            auto elapsed_usec = etc.elapsed_micro();
            if (elapsed_usec >
                published_response_time *
                    Settings::getInstance()->ProximityCheckResponseTimeMultiplier)
            {
                throw LibLogicalAccessException(
                    "Proximity check failed. Took " + std::to_string(elapsed_usec) +
                    " instead of " +
                    std::to_string(
                        published_response_time *
                        Settings::getInstance()->ProximityCheckResponseTimeMultiplier));
            }
        }

        // Card send us back exactly as many bytes as we sent it.
        EXCEPTION_ASSERT_WITH_LOG(resp.getData().size() == chunk_size,
                                  LibLogicalAccessException,
                                  "Unexpected number of bytes in response");
        rnd_r.insert(rnd_r.end(), resp.getData().begin(), resp.getData().end());

        MAC_SOURCE.insert(MAC_SOURCE.end(), resp.getData().begin(), resp.getData().end());
        MAC_SOURCE.insert(MAC_SOURCE.end(), rnd_c.begin() + byte_transferred,
                          rnd_c.begin() + byte_transferred + chunk_size);

        byte_transferred += chunk_size;
    }

    // Verify PC
    auto mac = DESFireEV2Crypto::truncateMAC(
        openssl::CMACCrypto::cmac(key->getData(), "aes", MAC_SOURCE));
    resp = transmit_plain(DFEV2_INS_VERIFY_PC, mac);

    // Now verify the MAC sent by the card.
    // This mac is computed over the same data, except the first byte where the card
    // uses 0x90 instead of 0xFD
    MAC_SOURCE[0]  = 0x90;
    auto mac_verif = DESFireEV2Crypto::truncateMAC(
        openssl::CMACCrypto::cmac(key->getData(), "aes", MAC_SOURCE));

    // The verif should match what the card sent us, otherwise PC check fails.
    EXCEPTION_ASSERT_WITH_LOG(mac_verif == resp.getData(), LibLogicalAccessException,
                              "Proximity Check failed. Cannot verify PICC provided MAC");
}

ByteVector DESFireEV2ISO7816Commands::commitTransaction(bool return_tmac)
{
    ByteVector data;
    data.push_back(return_tmac ? 0x01 : 0x00);
    return transmit(DF_COMMIT_TRANSACTION, data).getData();
}

ByteVector DESFireEV2ISO7816Commands::commitReaderID(ByteVector readerid)
{
    return transmit(DFEV2_INS_COMMIT_READERID, readerid).getData();
}

void DESFireEV2ISO7816Commands::restoreTransfer(unsigned char target_fileno, unsigned char source_fileno)
{
    ByteVector data;
    data.push_back(target_fileno);
    data.push_back(source_fileno);
    transmit(DFEV2_INS_RESTORE_TRANSFER, data);
}

ByteVector DESFireEV2ISO7816Commands::readSignature(unsigned char address)
{
    ByteVector params;
    params.push_back(address);
    std::shared_ptr<DESFireCrypto> crypto = getDESFireChip()->getCrypto();

    ISO7816Response r;
    if (crypto->d_sessionKey.size() > 0)
    {
        r = transmit_full(DFEV2_INS_READ_SIG, {}, params);
    }
    else
    {
        r = transmit_plain(DFEV2_INS_READ_SIG, params);
    }

    return r.getData();
}

bool DESFireEV2ISO7816Commands::performECCOriginalityCheck()
{
    ByteVector pubkey = { 0x04, 0x1D, 0xB4, 0x6C, 0x14, 0x5D, 0x0A, 0x36, 0x53, 0x9C, 0x65, 0x44,
        0xBD, 0x6D, 0x9B, 0x0A, 0xA6, 0x2F, 0xF9, 0x1E, 0xC4, 0x8C, 0xBC, 0x6A, 0xBA, 0xE3, 0x6E,
        0x00, 0x89, 0xA4, 0x6F, 0x0D, 0x08, 0xC8, 0xA7, 0x15, 0xEA, 0x40, 0xA6, 0x33, 0x13, 0xB9,
        0x2E, 0x90, 0xDD, 0xC1, 0x73, 0x02, 0x30, 0xE0, 0x45, 0x8A, 0x33, 0x27, 0x6F, 0xB7, 0x43 };
    
    ByteVector sig = readSignature(0x00);
    return SignatureHelper::verify_ecdsa_secp224r1(getDESFireChip()->getChipIdentifier(), sig, pubkey);
}