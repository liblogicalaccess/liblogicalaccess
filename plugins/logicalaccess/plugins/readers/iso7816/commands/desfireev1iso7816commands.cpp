/**
 * \file desfireev1iso7816commands.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief DESFireEV1 ISO7816 commands.
 */

#include <logicalaccess/plugins/readers/iso7816/commands/desfireev1iso7816commands.hpp>
#include <logicalaccess/plugins/cards/desfire/desfirechip.hpp>
#include <logicalaccess/plugins/cards/samav2/samav2commands.hpp>
#include <openssl/rand.h>
#include <logicalaccess/iks/IslogKeyServer.hpp>
#include <logicalaccess/plugins/llacommon/settings.hpp>
#include <logicalaccess/cards/IKSStorage.hpp>
#include <chrono>
#include <thread>
#include <logicalaccess/plugins/llacommon/logs.hpp>
#include <logicalaccess/plugins/crypto/aes_cipher.hpp>
#include <logicalaccess/plugins/crypto/aes_symmetric_key.hpp>
#include <logicalaccess/plugins/crypto/aes_initialization_vector.hpp>
#include <logicalaccess/plugins/crypto/des_cipher.hpp>
#include <logicalaccess/plugins/crypto/des_symmetric_key.hpp>
#include <logicalaccess/plugins/crypto/des_initialization_vector.hpp>
#include <logicalaccess/plugins/cards/samav2/samav2chip.hpp>
#include <logicalaccess/plugins/cards/desfire/desfireev1location.hpp>
#include <logicalaccess/cards/samkeystorage.hpp>
#include <logicalaccess/plugins/cards/samav2/samcommands.hpp>
#include <logicalaccess/plugins/cards/desfire/nxpav2keydiversification.hpp>
#include <logicalaccess/myexception.hpp>
#include <cassert>
#include <logicalaccess/iks/RemoteCrypto.hpp>
#include <logicalaccess/dynlibrary/librarymanager.hpp>
#include <logicalaccess/services/aes_crypto_service.hpp>
#include <logicalaccess/rtti_dump.hpp>

namespace logicalaccess
{
DESFireEV1ISO7816Commands::DESFireEV1ISO7816Commands()
    : DESFireISO7816Commands(CMD_DESFIREEV1ISO7816)
{
}

DESFireEV1ISO7816Commands::DESFireEV1ISO7816Commands(std::string ct)
    : DESFireISO7816Commands(ct)
{
}

DESFireEV1ISO7816Commands::~DESFireEV1ISO7816Commands()
{
}

unsigned int DESFireEV1ISO7816Commands::getFreeMem()
{
    unsigned int freemem = 0;

    ByteVector r = transmit(DFEV1_INS_FREE_MEM);
    r.resize(r.size() - 2);

    if (r.size() == 3)
    {
        freemem = (r[2] << 16) | (r[1] << 8) | r[0];
    }

    return freemem;
}

std::vector<DFName> DESFireEV1ISO7816Commands::getDFNames()
{
    std::vector<DFName> dfnames;
    unsigned char err;
    unsigned char cmd = DFEV1_INS_GET_DF_NAMES;

    do
    {
        ByteVector r = transmit(cmd);
        err          = r.back();
        r.resize(r.size() - 2);
        cmd = DF_INS_ADDITIONAL_FRAME;

        if (r.size() == 0)
            return dfnames;

        DFName dfname;
        memset(&dfname, 0x00, sizeof(dfname));
        dfname.AID = DESFireLocation::convertAidToUInt(r);
        dfname.FID = static_cast<unsigned short>((r[4] << 8) | r[3]);
        memcpy(dfname.DF_Name, &r[5], r.size() - 5);
        dfnames.push_back(dfname);
    } while (err == DF_INS_ADDITIONAL_FRAME);

    return dfnames;
}

std::vector<unsigned short> DESFireEV1ISO7816Commands::getISOFileIDs()
{
    std::vector<unsigned short> fileids;
    unsigned char err;
    unsigned char cmd = DFEV1_INS_GET_ISO_FILE_IDS;
    ByteVector fullr;

    do
    {
        ByteVector r = transmit(cmd);
        err          = r.back();
        r.resize(r.size() - 2);
        cmd = DF_INS_ADDITIONAL_FRAME;

        fullr.insert(fullr.end(), r.begin(), r.end());
    } while (err == DF_INS_ADDITIONAL_FRAME);

    for (unsigned int i = 0; i < fullr.size(); i += 2)
    {
        unsigned short fid = static_cast<unsigned short>((fullr[i + 1] << 8) | fullr[i]);
        fileids.push_back(fid);
    }

    return fileids;
}

void DESFireEV1ISO7816Commands::createApplication(
    unsigned int aid, DESFireKeySettings settings, unsigned char maxNbKeys,
    DESFireKeyType cryptoMethod, FidSupport fidSupported, unsigned short isoFID,
    ByteVector isoDFName)
{
    ByteVector command;
    std::shared_ptr<DESFireCrypto> crypto = getDESFireChip()->getCrypto();

    DESFireLocation::convertUIntToAid(aid, command);
    command.push_back(static_cast<unsigned char>(settings));
    command.push_back(
        static_cast<unsigned char>((maxNbKeys & 0x0f) | fidSupported | cryptoMethod));

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
    crypto->createApplication(aid, 1, maxNbKeys, cryptoMethod);
}

void DESFireEV1ISO7816Commands::getKeySettings(DESFireKeySettings &settings,
                                               unsigned char &maxNbKeys,
                                               DESFireKeyType &keyType)
{
    ByteVector r = transmit(DF_INS_GET_KEY_SETTINGS);
    r.resize(r.size() - 2);

    if (r.size() == 0)
        THROW_EXCEPTION_WITH_LOG(std::runtime_error,
                                 "getKeySettings did not return proper informations");

    settings  = static_cast<DESFireKeySettings>(r[0]);
    maxNbKeys = r[1] & 0x0F;

    if ((r[1] & DF_KEY_3K3DES) == DF_KEY_3K3DES)
    {
        keyType = DF_KEY_3K3DES;
    }
    else if ((r[1] & DF_KEY_AES) == DF_KEY_AES)
    {
        keyType = DF_KEY_AES;
    }
    else
    {
        keyType = DF_KEY_DES;
    }
}

ByteVector DESFireEV1ISO7816Commands::getCardUID()
{
    ByteVector result = transmit_nomacv(DFEV1_INS_GET_CARD_UID);
    if (result.size() >= 2)
    {
        result.resize(result.size() - 2);
        std::shared_ptr<DESFireCrypto> crypto = getDESFireChip()->getCrypto();

        crypto->initBuf();
        crypto->appendDecipherData(result);
        return crypto->desfireDecrypt(7);
    }
    THROW_EXCEPTION_WITH_LOG(std::runtime_error,
                             "Incorrect result when request card UID");
}

void DESFireEV1ISO7816Commands::createStdDataFile(unsigned char fileno,
                                                  EncryptionMode comSettings,
                                                  const DESFireAccessRights &accessRights,
                                                  unsigned int fileSize,
                                                  unsigned short isoFID)
{
    ByteVector command;
    short ar = AccessRightsInMemory(accessRights);

    command.push_back(static_cast<unsigned char>(fileno));
    if (isoFID != 0)
    {
        command.push_back(static_cast<unsigned char>(isoFID & 0xff));
        command.push_back(static_cast<unsigned char>(
            static_cast<unsigned short>(isoFID & 0xff00) >> 8));
    }
    command.push_back(static_cast<unsigned char>(comSettings));
    BufferHelper::setUShort(command, ar);
    command.push_back(static_cast<unsigned char>(fileSize & 0xff));
    command.push_back(
        static_cast<unsigned char>(static_cast<unsigned short>(fileSize & 0xff00) >> 8));
    command.push_back(
        static_cast<unsigned char>(static_cast<unsigned int>(fileSize & 0xff0000) >> 16));

    transmit(DF_INS_CREATE_STD_DATA_FILE, command);
}

void DESFireEV1ISO7816Commands::createBackupFile(unsigned char fileno,
                                                 EncryptionMode comSettings,
                                                 const DESFireAccessRights &accessRights,
                                                 unsigned int fileSize,
                                                 unsigned short isoFID)
{
    ByteVector command;
    short ar = AccessRightsInMemory(accessRights);

    command.push_back(static_cast<unsigned char>(fileno));
    if (isoFID != 0)
    {
        command.push_back(static_cast<unsigned char>(isoFID & 0xff));
        command.push_back(static_cast<unsigned char>(
            static_cast<unsigned short>(isoFID & 0xff00) >> 8));
    }
    command.push_back(static_cast<unsigned char>(comSettings));
    BufferHelper::setUShort(command, ar);
    command.push_back(static_cast<unsigned char>(fileSize & 0xff));
    command.push_back(
        static_cast<unsigned char>(static_cast<unsigned short>(fileSize & 0xff00) >> 8));
    command.push_back(
        static_cast<unsigned char>(static_cast<unsigned int>(fileSize & 0xff0000) >> 16));

    transmit(DF_INS_CREATE_BACKUP_DATA_FILE, command);
}

void DESFireEV1ISO7816Commands::createLinearRecordFile(
    unsigned char fileno, EncryptionMode comSettings,
    const DESFireAccessRights &accessRights, unsigned int fileSize,
    unsigned int maxNumberOfRecords, unsigned short isoFID)
{
    ByteVector command;
    short ar = AccessRightsInMemory(accessRights);

    command.push_back(static_cast<unsigned char>(fileno));
    if (isoFID != 0)
    {
        command.push_back(static_cast<unsigned char>(isoFID & 0xff));
        command.push_back(static_cast<unsigned char>(
            static_cast<unsigned short>(isoFID & 0xff00) >> 8));
    }
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

void DESFireEV1ISO7816Commands::createCyclicRecordFile(
    unsigned char fileno, EncryptionMode comSettings,
    const DESFireAccessRights &accessRights, unsigned int fileSize,
    unsigned int maxNumberOfRecords, unsigned short isoFID)
{
    ByteVector command;
    short ar = AccessRightsInMemory(accessRights);

    command.push_back(static_cast<unsigned char>(fileno));
    if (isoFID != 0)
    {
        command.push_back(static_cast<unsigned char>(isoFID & 0xff));
        command.push_back(static_cast<unsigned char>(
            static_cast<unsigned short>(isoFID & 0xff00) >> 8));
    }
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

void DESFireEV1ISO7816Commands::iso_selectFile(unsigned short fid)
{
    ByteVector data;
    data.push_back(
        static_cast<unsigned char>(static_cast<unsigned short>(fid & 0xff00) >> 8));
    data.push_back(static_cast<unsigned char>(fid & 0xff));

    getISO7816ReaderCardAdapter()->sendAPDUCommand(
        DFEV1_CLA_ISO_COMPATIBLE, ISO7816_INS_SELECT_FILE, 0x00, 0x0C,
        static_cast<unsigned char>(data.size()), data);
}

ByteVector DESFireEV1ISO7816Commands::iso_readRecords(unsigned short fid,
                                                      unsigned char start_record,
                                                      DESFireRecords record_number)
{
    unsigned char p1 = start_record;
    unsigned char p2 = record_number & 0x0f;
    if (fid != 0)
    {
        p2 += static_cast<unsigned char>((fid & 0xff) << 3);
    }

    ByteVector result = getISO7816ReaderCardAdapter()->sendAPDUCommand(
        DFEV1_CLA_ISO_COMPATIBLE, ISO7816_INS_READ_RECORDS, p1, p2, 0x00);

    return ByteVector(result.begin(), result.end() - 2);
}

void DESFireEV1ISO7816Commands::iso_appendrecord(const ByteVector &data,
                                                 unsigned short fid)
{
    unsigned char p1 = 0x00;
    unsigned char p2 = 0x00;
    if (fid != 0)
    {
        p2 += static_cast<unsigned char>((fid & 0xff) << 3);
    }

    getISO7816ReaderCardAdapter()->sendAPDUCommand(
        DFEV1_CLA_ISO_COMPATIBLE, ISO7816_INS_APPEND_RECORD, p1, p2,
        static_cast<unsigned char>(data.size()), data);
}

ByteVector DESFireEV1ISO7816Commands::iso_getChallenge(unsigned int length)
{
    ByteVector result = getISO7816ReaderCardAdapter()->sendAPDUCommand(
        DFEV1_CLA_ISO_COMPATIBLE, ISO7816_INS_GET_CHALLENGE, 0x00, 0x00,
        static_cast<unsigned char>(length));

    if (result[result.size() - 2] == 0x90 && result[result.size() - 1] == 0x00)
    {
        return ByteVector(result.begin(), result.end() - 2);
    }
    return result;
}

void DESFireEV1ISO7816Commands::iso_externalAuthenticate(DESFireISOAlgorithm algorithm,
                                                         bool isMasterCardKey,
                                                         unsigned char keyno,
                                                         const ByteVector &data)
{
    unsigned char p1 = static_cast<unsigned char>(algorithm);
    unsigned char p2 = 0x00;
    if (!isMasterCardKey)
    {
        p2 = static_cast<unsigned char>(0x80 + (keyno & 0x0F));
    }

    getISO7816ReaderCardAdapter()->sendAPDUCommand(
        DFEV1_CLA_ISO_COMPATIBLE, ISO7816_INS_EXTERNAL_AUTHENTICATE, p1, p2,
        static_cast<unsigned char>(data.size()), data);
}

ByteVector DESFireEV1ISO7816Commands::iso_internalAuthenticate(
    DESFireISOAlgorithm algorithm, bool isMasterCardKey, unsigned char keyno,
    const ByteVector &RPCD2, unsigned int length)
{
    unsigned char p1 = static_cast<unsigned char>(algorithm);
    unsigned char p2 = 0x00;
    if (!isMasterCardKey)
    {
        p2 = static_cast<unsigned char>(0x80 + (keyno & 0x0F));
    }

    ByteVector result = getISO7816ReaderCardAdapter()->sendAPDUCommand(
        DFEV1_CLA_ISO_COMPATIBLE, ISO7816_INS_INTERNAL_AUTHENTICATE, p1, p2,
        static_cast<unsigned char>(RPCD2.size()), RPCD2,
        static_cast<unsigned char>(length));

    if (result[result.size() - 2] == 0x90 && result[result.size() - 1] == 0x00)
    {
        return ByteVector(result.begin(), result.end() - 2);
    }
    return result;
}

void DESFireEV1ISO7816Commands::authenticate(unsigned char keyno,
                                             std::shared_ptr<DESFireKey> key)
{
    if (!key)
    {
        key = DESFireCrypto::getDefaultKey(DF_KEY_DES);
    }

    std::shared_ptr<DESFireCrypto> crypto = getDESFireChip()->getCrypto();
    crypto->setKey(crypto->d_currentAid, 0, keyno, key);

    // Get the appropriate authentification method and algorithm according to the key type
    // (for 3DES we use legacy method instead of ISO).

    if (std::dynamic_pointer_cast<SAMKeyStorage>(key->getKeyStorage()) && !getSAMChip())
        THROW_EXCEPTION_WITH_LOG(
            LibLogicalAccessException,
            "SAMKeyStorage set on the key but not SAM reader has been set.");

    if (std::dynamic_pointer_cast<SAMKeyStorage>(key->getKeyStorage()) &&
        !std::dynamic_pointer_cast<SAMKeyStorage>(key->getKeyStorage())->getDumpKey() &&
        key->getKeyType() != DF_KEY_DES)
    {
        if (key->getKeyType() == DF_KEY_3K3DES)
            sam_iso_authenticate(key, DF_ALG_3K3DES,
                                 (crypto->d_currentAid == 0 && keyno == 0), keyno);
        else
            sam_iso_authenticate(key, DF_ALG_AES,
                                 (crypto->d_currentAid == 0 && keyno == 0), keyno);
    }
    else if (key->getKeyStorage()->getType() == KST_SERVER)
    {
        if (key->getKeyType() == DF_KEY_AES)
            iks_iso_authenticate(key, (crypto->d_currentAid == 0 && keyno == 0), keyno);
        else
        {
            THROW_EXCEPTION_WITH_LOG(
                LibLogicalAccessException,
                "Combination of KeyStorage + KeyType is not supported.");
        }
    }
    else if (key->getKeyStorage()->getType() == KST_PKCS)
    {
        if (key->getKeyType() == DF_KEY_AES)
            pkcs_iso_authenticate(key, (crypto->d_currentAid == 0 && keyno == 0), keyno);
        else
        {
            THROW_EXCEPTION_WITH_LOG(
                LibLogicalAccessException,
                "Combination of KeyStorage + KeyType is not supported.");
        }
    }
    else
    {
        switch (key->getKeyType())
        {
        case DF_KEY_DES: DESFireISO7816Commands::authenticate(keyno, key); break;

        case DF_KEY_3K3DES:
            iso_authenticate(key, DF_ALG_3K3DES,
                             (crypto->d_currentAid == 0 && keyno == 0), keyno);
            break;

        case DF_KEY_AES:
            iso_authenticate(key, DF_ALG_AES, (crypto->d_currentAid == 0 && keyno == 0),
                             keyno);
            break;
        }
    }
    onAuthenticated();
}

void DESFireEV1ISO7816Commands::sam_iso_authenticate(std::shared_ptr<DESFireKey> key,
                                                     DESFireISOAlgorithm algorithm,
                                                     bool isMasterCardKey,
                                                     unsigned char keyno)
{
    unsigned char p1 = 0x00; // 0x02 if selectApplication has been proceed
    ByteVector diversify;
    std::shared_ptr<DESFireCrypto> crypto = getDESFireChip()->getCrypto();

    if (key->getKeyDiversification())
    {
        key->getKeyDiversification()->initDiversification(
            crypto->getIdentifier(), crypto->d_currentAid, key, keyno, diversify);
    }

    ByteVector apduresult;

    ByteVector RPICC1 = iso_getChallenge(16);

    ByteVector data(2 + RPICC1.size());

    if (!std::dynamic_pointer_cast<SAMKeyStorage>(key->getKeyStorage()))
        THROW_EXCEPTION_WITH_LOG(
            LibLogicalAccessException,
            "DESFireKey need a SAMKeyStorage to proceed a SAM ISO Authenticate.");

    if (getSAMChip()->getCardType() == "SAM_AV2" &&
        !std::dynamic_pointer_cast<NXPAV2KeyDiversification>(
            key->getKeyDiversification()))
    {
        LOG(LogLevel::INFOS) << "Start AuthenticationPICC in purpose to fix SAM state "
                                "(NXP SAM Documentation 3.5)";

        try
        {
            ByteVector randomAuthenticatePICC = {
                0x80, 0x0a, 0x00, 0x00, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            std::dynamic_pointer_cast<SAMCommands<KeyEntryAV2Information, SETAV2>>(
                getSAMChip()->getCommands())
                ->transmit(randomAuthenticatePICC);
        }
        catch (std::exception &)
        {
        }

        try
        {
            // Cancel SAM authentication with dummy command, but ignore return
            ByteVector cmd_vector = {0x80, 0xaf, 0x00, 0x00, 0x00};
            std::dynamic_pointer_cast<SAMCommands<KeyEntryAV2Information, SETAV2>>(
                getSAMChip()->getCommands())
                ->transmit(cmd_vector);
        }
        catch (std::exception &)
        {
        }
    }

    data[0] =
        std::dynamic_pointer_cast<SAMKeyStorage>(key->getKeyStorage())->getKeySlot();
    data[1] = key->getKeyVersion();
    memcpy(&data[0] + 2, &RPICC1[0], RPICC1.size());

    if (std::dynamic_pointer_cast<NXPKeyDiversification>(key->getKeyDiversification()))
    {
        p1 |= 0x01;
        if (std::dynamic_pointer_cast<NXPAV2KeyDiversification>(
                key->getKeyDiversification()))
            p1 |= 0x10;
        data.insert(data.end(), diversify.begin(), diversify.end());
    }

    unsigned char cmdp1[] = {0x80, 0x8e, p1, 0x00, (unsigned char)(data.size()), 0x00};
    ByteVector cmd_vector(cmdp1, cmdp1 + 6);
    cmd_vector.insert(cmd_vector.end() - 1, data.begin(), data.end());

    int trytoreconnect = 0;
    do
    {
        try
        {
            if (getSAMChip()->getCardType() == "SAM_AV1")
                apduresult = std::dynamic_pointer_cast<
                                 SAMCommands<KeyEntryAV1Information, SETAV1>>(
                                 getSAMChip()->getCommands())
                                 ->transmit(cmd_vector);
            else if (getSAMChip()->getCardType() == "SAM_AV2")
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
                ->reconnect();
        }
        ++trytoreconnect;
    } while (true);

    if (apduresult.size() <= 2 && apduresult[apduresult.size() - 2] != 0x90 &&
        apduresult[apduresult.size() - 2] != 0xaf)
        THROW_EXCEPTION_WITH_LOG(CardException, "sam_iso_authenticate P1 failed.");

    ByteVector encRPCD1RPICC1(apduresult.begin(), apduresult.end() - 2 - 16);
    ByteVector RPCD2(apduresult.end() - 16 - 2, apduresult.end() - 2);

    ByteVector encRPICC2RPCD2a;
    try
    {
        iso_externalAuthenticate(algorithm, isMasterCardKey, keyno, encRPCD1RPICC1);
        encRPICC2RPCD2a =
            iso_internalAuthenticate(algorithm, isMasterCardKey, keyno, RPCD2, 2 * 16);
    }
    catch (...)
    {
        std::exception_ptr eptr = std::current_exception();
        // Cancel SAM authentication with dummy command, but ignore return
        try
        {
            unsigned char resetcmd[] = {0x80, 0xaf, 0x00, 0x00, 0x00};
            cmd_vector.assign(resetcmd, resetcmd + 5);
            if (getSAMChip()->getCardType() == "SAM_AV1")
                apduresult = std::dynamic_pointer_cast<
                                 SAMCommands<KeyEntryAV1Information, SETAV1>>(
                                 getSAMChip()->getCommands())
                                 ->transmit(cmd_vector);
            else if (getSAMChip()->getCardType() == "SAM_AV2")
                apduresult = std::dynamic_pointer_cast<
                                 SAMCommands<KeyEntryAV2Information, SETAV2>>(
                                 getSAMChip()->getCommands())
                                 ->transmit(cmd_vector);
        }
        catch (std::exception &)
        {
        }

        std::rethrow_exception(eptr);
    }

    if (encRPICC2RPCD2a.size() < 1)
        THROW_EXCEPTION_WITH_LOG(CardException,
                                 "sam_iso_authenticate wrong internal data.");

    unsigned char cmdp2[] = {0x80, 0x8e, 0x00, 0x00,
                             (unsigned char)(encRPICC2RPCD2a.size())};
    cmd_vector.assign(cmdp2, cmdp2 + 5);
    cmd_vector.insert(cmd_vector.end(), encRPICC2RPCD2a.begin(), encRPICC2RPCD2a.end());
    if (getSAMChip()->getCardType() == "SAM_AV1")
        apduresult =
            std::dynamic_pointer_cast<SAMCommands<KeyEntryAV1Information, SETAV1>>(
                getSAMChip()->getCommands())
                ->transmit(cmd_vector);
    else if (getSAMChip()->getCardType() == "SAM_AV2")
        apduresult =
            std::dynamic_pointer_cast<SAMCommands<KeyEntryAV2Information, SETAV2>>(
                getSAMChip()->getCommands())
                ->transmit(cmd_vector);
    if (apduresult.size() <= 2 && apduresult[apduresult.size() - 2] != 0x90 &&
        apduresult[apduresult.size() - 2] != 0x00)
        THROW_EXCEPTION_WITH_LOG(CardException, "sam_iso_authenticate P2 failed.");

    crypto->d_currentKeyNo = keyno;
    if (getSAMChip()->getCardType() == "SAM_AV1")
        crypto->d_sessionKey =
            std::dynamic_pointer_cast<SAMCommands<KeyEntryAV1Information, SETAV1>>(
                getSAMChip()->getCommands())
                ->dumpSessionKey();
    else if (getSAMChip()->getCardType() == "SAM_AV2")
        crypto->d_sessionKey =
            std::dynamic_pointer_cast<SAMCommands<KeyEntryAV2Information, SETAV2>>(
                getSAMChip()->getCommands())
                ->dumpSessionKey();
    crypto->d_auth_method = CM_ISO;

    LOG(LogLevel::INFOS) << "Session key length: " << crypto->d_sessionKey.size();

    if (key->getKeyType() == DF_KEY_3K3DES || key->getKeyType() == DF_KEY_DES)
    {
        crypto->d_cipher.reset(new openssl::DESCipher());
        crypto->d_block_size = 8;
        crypto->d_mac_size   = 8;
    }
    else
    {
        crypto->d_cipher.reset(new openssl::AESCipher());
        crypto->d_block_size = 16;
        crypto->d_mac_size   = 8;
    }
    crypto->d_lastIV.clear();
    crypto->d_lastIV.resize(crypto->d_block_size, 0x00);
}

void DESFireEV1ISO7816Commands::iso_authenticate(std::shared_ptr<DESFireKey> currentKey,
                                                 DESFireISOAlgorithm algorithm,
                                                 bool isMasterCardKey,
                                                 unsigned char keyno)
{
    unsigned char le;
    std::shared_ptr<DESFireKey> key = std::make_shared<DESFireKey>(*currentKey);
    std::shared_ptr<openssl::SymmetricCipher> cipher;
    std::shared_ptr<DESFireCrypto> crypto = getDESFireChip()->getCrypto();

    auto diversify = getKeyInformations(key, keyno);

    ByteVector keydiv;
    crypto->getKey(key, diversify, keydiv);

    if (algorithm == DF_ALG_2K3DES)
    {
        le = 8;
        cipher.reset(new openssl::DESCipher());
    }
    else if (algorithm == DF_ALG_3K3DES)
    {
        le = 16;
        cipher.reset(new openssl::DESCipher());
    }
    else
    {
        le = 16;
 cipher = std::make_shared<openssl::AESCipher>();
    }
    std::shared_ptr<openssl::SymmetricKey> isokey;
    std::shared_ptr<openssl::InitializationVector> iv;

    if ( std::dynamic_pointer_cast<openssl::AESCipher>(cipher) != nullptr)
    {
        isokey.reset(new openssl::AESSymmetricKey(
            openssl::AESSymmetricKey::createFromData(keydiv)));
        iv.reset(new openssl::AESInitializationVector(
            openssl::AESInitializationVector::createNull()));
    }
    else
    {
        isokey.reset(new openssl::DESSymmetricKey(
            openssl::DESSymmetricKey::createFromData(keydiv)));
        iv.reset(new openssl::DESInitializationVector(
            openssl::DESInitializationVector::createNull()));
    }

    ByteVector RPICC1 = iso_getChallenge(le);

    ByteVector RPCD1;
    RPCD1.resize(le);

    EXCEPTION_ASSERT_WITH_LOG(RAND_status() == 1, LibLogicalAccessException,
                              "Insufficient enthropy source");
    if (RAND_bytes(&RPCD1[0], static_cast<int>(RPCD1.size())) != 1)
    {
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "Cannot retrieve cryptographically strong bytes");
    }
    ByteVector makecrypt1;
    makecrypt1.insert(makecrypt1.end(), RPCD1.begin(), RPCD1.end());
    makecrypt1.insert(makecrypt1.end(), RPICC1.begin(), RPICC1.end());
    ByteVector cryptogram;
    cipher->cipher(makecrypt1, cryptogram, *isokey.get(), *iv.get(), false);
    if (std::dynamic_pointer_cast<openssl::AESCipher>(cipher))
    {
        iv.reset(new openssl::AESInitializationVector(
            openssl::AESInitializationVector::createFromData(
                ByteVector(cryptogram.end() - iv->data().size(), cryptogram.end()))));
    }
    else
    {
        iv.reset(new openssl::DESInitializationVector(
            openssl::DESInitializationVector::createFromData(
                ByteVector(cryptogram.end() - iv->data().size(), cryptogram.end()))));
    }
    iso_externalAuthenticate(algorithm, isMasterCardKey, keyno, cryptogram);

    ByteVector RPCD2;
    RPCD2.resize(le);

    EXCEPTION_ASSERT_WITH_LOG(RAND_status() == 1, LibLogicalAccessException,
                              "Insufficient enthropy source");
    if (RAND_bytes(&RPCD2[0], static_cast<int>(RPCD2.size())) != 1)
    {
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "Cannot retrieve cryptographically strong bytes");
    }
    cryptogram =
        iso_internalAuthenticate(algorithm, isMasterCardKey, keyno, RPCD2, 2 * le);

    if (cryptogram.size() < 1)
        THROW_EXCEPTION_WITH_LOG(CardException, "iso_authenticate wrong internal data.");

    ByteVector response;
    cipher->decipher(cryptogram, response, *isokey.get(), *iv.get(), false);

    ByteVector RPICC2 = ByteVector(response.begin(), response.begin() + le);
    ByteVector RPCD2a = ByteVector(response.begin() + le, response.end());

    EXCEPTION_ASSERT_WITH_LOG(RPCD2 == RPCD2a, CardException,
                              "Integrity error : host part of mutual authentication");

    crypto->d_currentKeyNo = keyno;

    crypto->d_sessionKey.clear();
    crypto->d_auth_method = CM_ISO;
    if (algorithm == DF_ALG_2K3DES)
    {
        if (ByteVector(keydiv.begin(), keydiv.begin() + 8) ==
            ByteVector(keydiv.begin() + 8, keydiv.begin() + 8 + 8))
        {
            crypto->d_sessionKey.insert(crypto->d_sessionKey.end(), RPCD1.begin(),
                                        RPCD1.begin() + 4);
            crypto->d_sessionKey.insert(crypto->d_sessionKey.end(), RPICC2.begin(),
                                        RPICC2.begin() + 4);
            crypto->d_sessionKey.insert(crypto->d_sessionKey.end(), RPCD1.begin(),
                                        RPCD1.begin() + 4);
            crypto->d_sessionKey.insert(crypto->d_sessionKey.end(), RPICC2.begin(),
                                        RPICC2.begin() + 4);
        }
        else
        {
            crypto->d_sessionKey.insert(crypto->d_sessionKey.end(), RPCD1.begin(),
                                        RPCD1.begin() + 4);
            crypto->d_sessionKey.insert(crypto->d_sessionKey.end(), RPICC2.begin(),
                                        RPICC2.begin() + 4);
            crypto->d_sessionKey.insert(crypto->d_sessionKey.end(), RPCD1.begin() + 4,
                                        RPCD1.begin() + 8);
            crypto->d_sessionKey.insert(crypto->d_sessionKey.end(), RPICC2.begin() + 4,
                                        RPICC2.begin() + 8);
        }

        crypto->d_cipher.reset(new openssl::DESCipher());
        crypto->d_authkey    = keydiv;
        crypto->d_block_size = 8;
        crypto->d_mac_size   = 8;
    }
    else if (algorithm == DF_ALG_3K3DES)
    {
        crypto->d_sessionKey.insert(crypto->d_sessionKey.end(), RPCD1.begin(),
                                    RPCD1.begin() + 4);
        crypto->d_sessionKey.insert(crypto->d_sessionKey.end(), RPICC2.begin(),
                                    RPICC2.begin() + 4);
        crypto->d_sessionKey.insert(crypto->d_sessionKey.end(), RPCD1.begin() + 6,
                                    RPCD1.begin() + 10);
        crypto->d_sessionKey.insert(crypto->d_sessionKey.end(), RPICC2.begin() + 6,
                                    RPICC2.begin() + 10);
        crypto->d_sessionKey.insert(crypto->d_sessionKey.end(), RPCD1.begin() + 12,
                                    RPCD1.begin() + 16);
        crypto->d_sessionKey.insert(crypto->d_sessionKey.end(), RPICC2.begin() + 12,
                                    RPICC2.begin() + 16);

        crypto->d_cipher.reset(new openssl::DESCipher());
        crypto->d_authkey    = keydiv;
        crypto->d_block_size = 8;
        crypto->d_mac_size   = 8;
    }
    else
    {
        crypto->d_sessionKey.insert(crypto->d_sessionKey.end(), RPCD1.begin(),
                                    RPCD1.begin() + 4);
        crypto->d_sessionKey.insert(crypto->d_sessionKey.end(), RPICC2.begin(),
                                    RPICC2.begin() + 4);
        crypto->d_sessionKey.insert(crypto->d_sessionKey.end(), RPCD1.begin() + 12,
                                    RPCD1.begin() + 16);
        crypto->d_sessionKey.insert(crypto->d_sessionKey.end(), RPICC2.begin() + 12,
                                    RPICC2.begin() + 16);

        crypto->d_cipher.reset(new openssl::AESCipher());
        crypto->d_authkey    = keydiv;
        crypto->d_block_size = 16;
        crypto->d_mac_size   = 8;
    }
    crypto->d_lastIV.clear();
    crypto->d_lastIV.resize(crypto->d_block_size, 0x00);

    onAuthenticated();
}

void DESFireEV1ISO7816Commands::authenticateISO(unsigned char keyno,
                                                DESFireISOAlgorithm algorithm)
{
    std::shared_ptr<DESFireCrypto> crypto = getDESFireChip()->getCrypto();
    crypto->d_auth_method                 = CM_LEGACY; // To prevent CMAC checking

    unsigned char random_len;
    if (algorithm == DF_ALG_2K3DES)
    {
        random_len = 8;
    }
    else
    {
        random_len = 16;
    }

    ByteVector data;
    data.push_back(keyno);

    ByteVector diversify;
    std::shared_ptr<DESFireKey> currentkey = crypto->getKey(0, keyno);
    if (currentkey->getKeyDiversification())
    {
        currentkey->getKeyDiversification()->initDiversification(
            crypto->getIdentifier(), crypto->d_currentAid, currentkey, keyno, diversify);
    }

    ByteVector encRndB = transmit_plain(DFEV1_INS_AUTHENTICATE_ISO, data);
    unsigned char err  = encRndB.back();
    encRndB.resize(encRndB.size() - 2);
    EXCEPTION_ASSERT_WITH_LOG(err == DF_INS_ADDITIONAL_FRAME, LibLogicalAccessException,
                              "No additional frame for ISO authentication.");

    ByteVector response =
        crypto->iso_authenticate_PICC1(keyno, diversify, encRndB, random_len);
    ByteVector encRndA1 = transmit_plain(DF_INS_ADDITIONAL_FRAME, response);
    encRndA1.resize(encRndA1.size() - 2);

    crypto->iso_authenticate_PICC2(keyno, encRndA1, random_len);
}


void DESFireEV1ISO7816Commands::iks_authenticateAES(std::shared_ptr<DESFireKey> key,
                                                    uint8_t keyno)
{
    std::shared_ptr<DESFireCrypto> crypto = getDESFireChip()->getCrypto();
    auto kst = std::dynamic_pointer_cast<IKSStorage>(key->getKeyStorage());
    assert(kst);

    auto remote_crypto = LibraryManager::getInstance()->getRemoteCrypto();

    ByteVector data;
    data.push_back(keyno);

    // Get cryptogram from card.
    ByteVector encRndB = transmit_plain(DFEV1_INS_AUTHENTICATE_AES, data);
    unsigned char err  = encRndB.back();
    encRndB.resize(encRndB.size() - 2);
    EXCEPTION_ASSERT_WITH_LOG(err == DF_INS_ADDITIONAL_FRAME, LibLogicalAccessException,
                              "No additional frame for ISO authentication.");

    bool out_success;
    ByteVector out_encrypted_cryptogram, out_auth_context_id;
    remote_crypto->aes_authenticate_step1(kst->getKeyIdentity(), encRndB,
                                          extract_iks_div_info(key, keyno), out_success,
                                          out_encrypted_cryptogram, out_auth_context_id);
    /*

        CMSG_DesfireAESAuth_Step1 req;
        req.set_key_uuid(kst->getKeyIdentity());
        req.set_encrypted_random_picc(BufferHelper::getStdString(encRndB));
        *req.mutable_diversification() = extract_iks_div_info(key, keyno);
        auto step1_response            = rpc_client.desfire_auth_aes_step1(req);

    */

    ByteVector encRndA1 =
        transmit_plain(DF_INS_ADDITIONAL_FRAME, out_encrypted_cryptogram);
    encRndA1.resize(encRndA1.size() - 2);
    /*
        CMSG_DesfireAuth_Step2 step2;
        step2.set_key_uuid(kst->getKeyIdentity());
        step2.set_auth_context_id(step1_response.auth_context_id());
        step2.set_picc_cryptogram(BufferHelper::getStdString(encRndA1));

        auto step2_response = rpc_client.desfire_auth_aes_step2(step2);*/

    ByteVector out_session_key, out_session_key_ref;
    remote_crypto->aes_authenticate_step2(kst->getKeyIdentity(), encRndA1,
                                          out_auth_context_id,
                                          extract_iks_div_info(key, keyno), out_success,
                                          out_session_key, out_session_key_ref);

    if (out_success)
    {
        if (!out_session_key.empty())
        {
            // Raw key material. Use as is.
            crypto->d_sessionKey =
                ByteVector(out_session_key.begin(), out_session_key.end());
        }
        else
        {
            // We received a KeyReference. We need to create an IKSCryptoWrapper
            // for use by DESFireCrypto so CMAC calculation, data decryption and stuff
            // like that gets rerouted through IKS.
            crypto->iks_wrapper_ = std::make_unique<IKSCryptoWrapper>();
            crypto->iks_wrapper_->remote_key_name =
                std::string(out_session_key_ref.begin(), out_session_key_ref.end());
        }
        crypto->d_currentKeyNo = keyno;
    }
    else
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "AES Authenticate PICC 2 Failed!");

    crypto->d_cipher.reset(new openssl::AESCipher());
    crypto->d_auth_method = CM_ISO;
    crypto->d_block_size  = 16;
    crypto->d_mac_size    = 8;
    crypto->d_lastIV.clear();
    crypto->d_lastIV.resize(crypto->d_block_size, 0x00);
}

void DESFireEV1ISO7816Commands::authenticateAES(unsigned char keyno)
{
    std::shared_ptr<DESFireCrypto> crypto = getDESFireChip()->getCrypto();
    crypto->d_auth_method                 = CM_LEGACY; // To prevent CMAC checking

    std::shared_ptr<DESFireKey> key = crypto->getKey(0, keyno);
    if (key && key->getKeyStorage()->getType() == KST_SERVER)
    {
        iks_authenticateAES(key, keyno);
        return;
    }

    ByteVector data;
    data.push_back(keyno);

    auto diversify = getKeyInformations(key, keyno);

    auto samKeyStorage = std::dynamic_pointer_cast<SAMKeyStorage>(key->getKeyStorage());
    if (samKeyStorage && !getSAMChip())
        THROW_EXCEPTION_WITH_LOG(
            LibLogicalAccessException,
            "SAMKeyStorage set on the key but no SAM reader has been set.");

    ByteVector encRndB = transmit_plain(DFEV1_INS_AUTHENTICATE_AES, data);
    unsigned char err  = encRndB.back();
    encRndB.resize(encRndB.size() - 2);
    EXCEPTION_ASSERT_WITH_LOG(err == DF_INS_ADDITIONAL_FRAME, LibLogicalAccessException,
                              "No additional frame for ISO authentication.");

    ByteVector response;
    if (samKeyStorage)
        response = sam_authenticate_p1(key, encRndB, diversify);
    else
        // response        = crypto->aes_authenticate_PICC1(keyno, diversify, encRndB);
        response        = crypto->aes_authenticate_PICC1_GENERIC(keyno, key, encRndB);
    ByteVector encRndA1 = transmit_plain(DF_INS_ADDITIONAL_FRAME, response);
    encRndA1.resize(encRndA1.size() - 2);

    if (samKeyStorage)
        sam_authenticate_p2(keyno, encRndA1);
    else
        // crypto->aes_authenticate_PICC2(keyno, encRndA1);
        crypto->aes_authenticate_PICC2_GENERIC(keyno, key, encRndA1);

    crypto->d_cipher.reset(new openssl::AESCipher());
    crypto->d_auth_method = CM_ISO;
    crypto->d_block_size  = 16;
    crypto->d_mac_size    = 8;
    crypto->d_lastIV.clear();
    crypto->d_lastIV.resize(crypto->d_block_size, 0x00);
}

ByteVector DESFireEV1ISO7816Commands::handleReadCmd(unsigned char cmd,
                                                    const ByteVector &data,
                                                    EncryptionMode /*mode*/)
{
    // EV1 always add MAC even in plain...
    return transmit_nomacv(cmd, data);
}

ByteVector DESFireEV1ISO7816Commands::handleReadData(unsigned char err,
                                                     const ByteVector &firstMsg,
                                                     unsigned int length,
                                                     EncryptionMode mode)
{
    ByteVector ret, data;
    std::shared_ptr<DESFireCrypto> crypto = getDESFireChip()->getCrypto();

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
            crypto->initBuf();
        }
    }

    while (err == DF_INS_ADDITIONAL_FRAME)
    {
        data = transmit_plain(DF_INS_ADDITIONAL_FRAME);
        err  = data.back();
        data.resize(data.size() - 2);
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
    case CM_PLAIN:
    {
        if (crypto->d_auth_method != CM_LEGACY)
        {
            if (!crypto->verifyMAC(true, ret))
            {
                THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                         "MAC data doesn't match.");
            }
            ret.resize(ret.size() - crypto->d_mac_size);
        }
    }
    break;

    case CM_MAC:
    {
        if (!crypto->verifyMAC(true, ret))
        {
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                     "MAC data doesn't match.");
        }
        ret.resize(ret.size() - crypto->d_mac_size);
    }
    break;
    case CM_ENCRYPT:
    {
        ret                        = crypto->desfireDecrypt(length);
        handle_read_data_last_sig_ = crypto->get_last_signature();
    }
    break;
    case CM_UNKNOWN: {
    }
    break;
    }

    return ret;
}

void DESFireEV1ISO7816Commands::handleWriteData(unsigned char cmd,
                                                const ByteVector &parameters,
                                                const ByteVector &data,
                                                EncryptionMode mode)
{
    std::shared_ptr<DESFireCrypto> crypto = getDESFireChip()->getCrypto();
    if (crypto->d_auth_method == CM_LEGACY)
        return DESFireISO7816Commands::handleWriteData(cmd, parameters, data, mode);

    crypto->initBuf();

    size_t DESFIRE_EV1_CLEAR_DATA_LENGTH_CHUNK = (DESFIREEV1_CLEAR_DATA_LENGTH_CHUNK - 8);

    ByteVector cmdBuffer, result;
    cmdBuffer.insert(cmdBuffer.end(), parameters.begin(), parameters.end());

    switch (mode)
    {
    case CM_PLAIN:
    {
        // Always calcul a MAC even if not used
        ByteVector apdu_command;
        apdu_command.push_back(cmd);
        apdu_command.insert(apdu_command.end(), parameters.begin(), parameters.end());
        apdu_command.insert(apdu_command.end(), data.begin(), data.end());
        crypto->generateMAC(cmd, apdu_command);

        cmdBuffer.insert(cmdBuffer.end(), data.begin(), data.end());
    }
    break;

    case CM_MAC:
    {
        ByteVector gdata;
        gdata.push_back(cmd);
        gdata.insert(gdata.end(), parameters.begin(), parameters.end());
        gdata.insert(gdata.end(), data.begin(), data.end());
        auto mac = crypto->generateMAC(cmd, gdata);

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

    unsigned char err;
    size_t pos = 0;
    do
    {
        // Send the data little by little
        size_t pkSize =
            ((cmdBuffer.size() - pos) >= (DESFIRE_EV1_CLEAR_DATA_LENGTH_CHUNK))
                ? (DESFIRE_EV1_CLEAR_DATA_LENGTH_CHUNK)
                : (cmdBuffer.size() - pos);

        auto command =
            ByteVector(cmdBuffer.begin() + pos, cmdBuffer.begin() + pos + pkSize);

        if (command.size() == 0)
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                     "Data requested but no more data to send.");

        result = transmit_plain(pos == 0 ? cmd : DF_INS_ADDITIONAL_FRAME, command);

        err = result.back();
        pos += pkSize;
    } while (err == DF_INS_ADDITIONAL_FRAME);

    if (err != 0x00)
    {
        char msgtmp[64];
        sprintf(msgtmp, "Unknown error: %x", err);
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, msgtmp);
    }

    if (result.size() > 2)
    {
        result.resize(result.size() - 2);
        if (!crypto->verifyMAC(true, result))
        {
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                     "MAC verification failed.");
        }
    }
}

ByteVector DESFireEV1ISO7816Commands::readData(unsigned char fileno, unsigned int offset,
                                               unsigned int length, EncryptionMode mode)
{
    ByteVector command(7), ret, result;

    command[0] = static_cast<unsigned char>(fileno);

    if (length == 0)
    {
        while (ret.size() == 0 ||
               (result.size() >= 2 && result[result.size() - 2] == 0x90 &&
                result[result.size() - 1] == 0xAF))
        {
            result            = handleReadCmd(DF_INS_READ_DATA, command, mode);
            ByteVector data   = result;
            unsigned char err = data.back();
            data.resize(data.size() - 2);
            data = handleReadData(err, data, 0, mode);
            ret.insert(ret.end(), data.begin(), data.end());
        }
    }
    else
    {
        // Currently we have some problems to read more than 253 bytes with an Omnikey
        // Reader.
        // So the read command is separated to some commands, 8 bytes aligned.

        for (size_t i = 0; i < length; i += 248)
        {
            size_t trunloffset = offset + i;
            size_t trunklength = ((length - i) > 248) ? 248 : (length - i);
            memcpy(&command[1], &trunloffset, 3);
            memcpy(&command[4], &trunklength, 3);

            result            = handleReadCmd(DF_INS_READ_DATA, command, mode);
            unsigned char err = result.back();
            result.resize(result.size() - 2);

            result =
                handleReadData(err, result, static_cast<unsigned int>(trunklength), mode);
            ret.insert(ret.end(), result.begin(), result.end());
        }
    }
    return ret;
}

ByteVector DESFireEV1ISO7816Commands::readRecords(unsigned char fileno,
                                                  unsigned int offset,
                                                  unsigned int length,
                                                  EncryptionMode mode)
{
    ByteVector command;

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

    ByteVector result = handleReadCmd(DF_INS_READ_RECORDS, command, mode);
    unsigned char err = result.back();
    result.resize(result.size() - 2);
    result = handleReadData(err, result, 0, mode);

    return result;
}

void DESFireEV1ISO7816Commands::changeFileSettings(
    unsigned char fileno, EncryptionMode comSettings,
    const DESFireAccessRights &accessRights, bool plain)
{
    std::shared_ptr<DESFireCrypto> crypto = getDESFireChip()->getCrypto();
    ByteVector command;
    short ar = AccessRightsInMemory(accessRights);

    command.push_back(static_cast<unsigned char>(comSettings));
    BufferHelper::setUShort(command, ar);

    if (!plain)
    {
        ByteVector param;
        param.push_back(DF_INS_CHANGE_FILE_SETTINGS);
        param.push_back(static_cast<unsigned char>(fileno));
        crypto->initBuf();
        command = crypto->desfireEncrypt(command, param);
    }
    unsigned char uf = static_cast<unsigned char>(fileno);
    command.insert(command.begin(), uf);

    if (crypto->d_auth_method == CM_LEGACY || plain)
    {
        transmit_plain(DF_INS_CHANGE_FILE_SETTINGS, command);
    }
    else
    {
        ByteVector dd = transmit_plain(DF_INS_CHANGE_FILE_SETTINGS, command);
        dd.resize(dd.size() - 2);
        if (!crypto->verifyMAC(true, dd))
        {
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                     "MAC verification failed.");
        }
    }
}

void DESFireEV1ISO7816Commands::changeKeySettings(DESFireKeySettings settings)
{
    std::shared_ptr<DESFireCrypto> crypto = getDESFireChip()->getCrypto();
    ByteVector command(1);
    command[0] = static_cast<unsigned char>(settings);

    ByteVector param(1);
    param[0] = DF_INS_CHANGE_KEY_SETTINGS;

    crypto->initBuf();
    ByteVector cryptogram = crypto->desfireEncrypt(command, param);

    if (crypto->d_auth_method == CM_LEGACY)
    {
        transmit_plain(DF_INS_CHANGE_KEY_SETTINGS, cryptogram);
    }
    else
    {
        ByteVector r = transmit_plain(DF_INS_CHANGE_KEY_SETTINGS, cryptogram);
        r.resize(r.size() - 2);
        if (!crypto->verifyMAC(true, r))
        {
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                     "MAC verification failed.");
        }
    }
}

void DESFireEV1ISO7816Commands::changeKey(unsigned char keyno,
                                          std::shared_ptr<DESFireKey> newkey)
{
    std::shared_ptr<DESFireCrypto> crypto = getDESFireChip()->getCrypto();
    std::shared_ptr<DESFireKey> key       = std::make_shared<DESFireKey>(*newkey);
    auto oldkey                           = crypto->getKey(0, keyno);

    bool samChangeKey = checkChangeKeySAMKeyStorage(keyno, oldkey, key);

    auto oldKeyDiversify = getKeyInformations(crypto->getKey(0, keyno), keyno);
    auto newKeyDiversify = getKeyInformations(key, keyno);

    unsigned char keynobyte = keyno;
    if (keyno == 0 && crypto->d_currentAid == 0)
    {
        keynobyte |= key->getKeyType();
    }

    ByteVector cryptogram;
    std::cout << "SESSION KEY: " << crypto->d_sessionKey << std::endl;
    std::cout << "LAST_IV: " << crypto->d_lastIV << std::endl;
    if (samChangeKey)
    {
        cryptogram = getChangeKeySAMCryptogram(keyno, key);
    }
    else if (key->getKeyStorage()->getType() == KST_SERVER)
    {
        cryptogram = getChangeKeyIKSCryptogram(keyno, key);
    }
    else
    {
        cryptogram =
            crypto->changeKey_PICC(keynobyte, oldKeyDiversify, key, newKeyDiversify);
    }

    std::cout << "CRYPTOGRAM: " << cryptogram << std::endl;

    ByteVector data;
    data.push_back(keynobyte);
    data.insert(data.end(), cryptogram.begin(), cryptogram.end());
    if (crypto->d_auth_method == CM_LEGACY)
    {
        transmit_plain(DF_INS_CHANGE_KEY, data);
    }
    else
    {
        ByteVector dd = transmit_plain(DF_INS_CHANGE_KEY, data);
        dd.resize(dd.size() - 2);

        // Don't check CMAC if master key.
        if (dd.size() > 0 && keyno != 0)
        {
            if (!crypto->verifyMAC(true, dd))
            {
                THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                         "MAC verification failed.");
            }
        }
    }
    crypto->setKey(crypto->d_currentAid, 0, keyno, newkey);
}

DESFireCommands::DESFireCardVersion DESFireEV1ISO7816Commands::getVersion()
{
    DESFireCardVersion dataVersion;
    ByteVector allresult;
    std::shared_ptr<DESFireCrypto> crypto = getDESFireChip()->getCrypto();

    memset(&dataVersion, 0x00, sizeof(dataVersion));

    ByteVector result = transmit_nomacv(DF_INS_GET_VERSION);

    if ((result.size() - 2) >= 7)
    {
        if (result[result.size() - 1] == DF_INS_ADDITIONAL_FRAME)
        {
            memcpy(&dataVersion, &result[0], 7);
            allresult.insert(allresult.end(), result.begin(), result.end() - 2);

            result = transmit_plain(DF_INS_ADDITIONAL_FRAME);

            if (result.size() - 2 >= 7)
            {
                if (result[result.size() - 1] == DF_INS_ADDITIONAL_FRAME)
                {
                    memcpy(reinterpret_cast<char *>(&dataVersion) + 7, &result[0], 7);
                    allresult.insert(allresult.end(), result.begin(), result.end() - 2);

                    result = transmit_plain(DF_INS_ADDITIONAL_FRAME);
                    if (result.size() - 2 >= 14)
                    {
                        if (result[result.size() - 1] == 0x00)
                        {
                            memcpy(reinterpret_cast<char *>(&dataVersion) + 14,
                                   &result[0], 14);
                            allresult.insert(allresult.end(), result.begin(),
                                             result.end() - 2);

                            if (crypto->d_auth_method != CM_LEGACY)
                            {
                                if (!crypto->verifyMAC(true, allresult))
                                {
                                    THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                                             "MAC verification failed.");
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return dataVersion;
}

std::vector<unsigned int> DESFireEV1ISO7816Commands::getApplicationIDs()
{
    std::vector<unsigned int> aids;
    std::shared_ptr<DESFireCrypto> crypto = getDESFireChip()->getCrypto();

    ByteVector result = transmit(DF_INS_GET_APPLICATION_IDS);

    if (result.size() < 2)
        THROW_EXCEPTION_WITH_LOG(std::runtime_error,
                                 "getApplicationIDs did not return proper informations");

    while (result[result.size() - 1] == DF_INS_ADDITIONAL_FRAME)
    {
        for (size_t i = 0; i < result.size() - 2; i += 3)
        {
            ByteVector aid(result.begin() + i, result.begin() + i + 3);
            aids.push_back(DESFireLocation::convertAidToUInt(aid));
        }

        result = transmit(DF_INS_ADDITIONAL_FRAME);
    }

    result.resize(result.size() - 2);

    for (size_t i = 0; i < result.size(); i += 3)
    {
        ByteVector aid(result.begin() + i, result.begin() + i + 3);
        aids.push_back(DESFireLocation::convertAidToUInt(aid));
    }

    return aids;
}

ByteVector DESFireEV1ISO7816Commands::getFileIDs()
{
    std::shared_ptr<DESFireCrypto> crypto = getDESFireChip()->getCrypto();
    ByteVector result                     = transmit(DF_INS_GET_FILE_IDS);
    result.resize(result.size() - 2);

    ByteVector files;
    for (size_t i = 0; i < result.size(); ++i)
    {
        files.push_back(result[i]);
    }

    return files;
}

void DESFireEV1ISO7816Commands::getValue(unsigned char fileno, EncryptionMode mode,
                                         unsigned int &value)
{
    ByteVector command;
    command.push_back(fileno);

    ByteVector result = handleReadCmd(DF_INS_GET_VALUE, command, mode);
    unsigned char err = result.back();
    result.resize(result.size() - 2);
    result = handleReadData(err, result, 4, mode);

    if (result.size() >= 4)
    {
        size_t offset = 0;
        value         = BufferHelper::getInt32(result, offset);
    }
}

void DESFireEV1ISO7816Commands::iso_selectApplication(ByteVector isoaid)
{
    getISO7816ReaderCardAdapter()->sendAPDUCommand(
        DFEV1_CLA_ISO_COMPATIBLE, ISO7816_INS_SELECT_FILE, SELECT_FILE_BY_AID, 0x00,
        static_cast<unsigned char>(isoaid.size()), isoaid);
}

void DESFireEV1ISO7816Commands::setConfiguration(bool formatCardEnabled,
                                                 bool randomIdEnabled)
{
    std::shared_ptr<DESFireCrypto> crypto = getDESFireChip()->getCrypto();
    ByteVector command(1);
    command[0] = ((formatCardEnabled) ? 0x00 : 0x01) | ((randomIdEnabled) ? 0x02 : 0x00);

    crypto->initBuf();
    ByteVector param;
    param.push_back(DFEV1_INS_SET_CONFIGURATION);
    param.push_back(0x00); // PICC App key config
    ByteVector encBuffer = crypto->desfireEncrypt(command, param);
    ByteVector buf;
    buf.push_back(0x00);
    buf.insert(buf.end(), encBuffer.begin(), encBuffer.end());
    transmit_plain(DFEV1_INS_SET_CONFIGURATION, buf);
}

void DESFireEV1ISO7816Commands::setConfiguration(std::shared_ptr<DESFireKey> defaultKey)
{
    if (defaultKey->getLength() < 24)
    {
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "The default key length must be 24-byte long.");
    }
    std::shared_ptr<DESFireCrypto> crypto = getDESFireChip()->getCrypto();

    ByteVector command = defaultKey->getBytes();
    command.resize(25);
    command[24] = defaultKey->getKeyVersion();

    crypto->initBuf();
    ByteVector param;
    param.push_back(DFEV1_INS_SET_CONFIGURATION);
    param.push_back(0x01); // Default keys Update
    ByteVector encBuffer = crypto->desfireEncrypt(command, param);
    ByteVector buf;
    buf.push_back(0x01);
    buf.insert(buf.end(), encBuffer.begin(), encBuffer.end());
    transmit_plain(DFEV1_INS_SET_CONFIGURATION, buf);
}

void DESFireEV1ISO7816Commands::setConfiguration(const ByteVector &ats)
{
    std::shared_ptr<DESFireCrypto> crypto = getDESFireChip()->getCrypto();
    ByteVector data                       = ats;
    ByteVector buf;
    buf.push_back(0x02);
    if (crypto->d_auth_method == CM_LEGACY)
    {
        short crc = DESFireCrypto::desfire_crc16(&data[0], data.size());
        data.push_back(static_cast<unsigned char>(crc & 0xff));
        data.push_back(static_cast<unsigned char>((crc & 0xff00) >> 8));
    }
    else
    {
        ByteVector calconbuf;
        calconbuf.push_back(DFEV1_INS_SET_CONFIGURATION);
        calconbuf.push_back(0x02);
        calconbuf.insert(calconbuf.end(), ats.begin(), ats.end());
        uint32_t crc = DESFireCrypto::desfire_crc32(&calconbuf[0], calconbuf.size());
        data.push_back(static_cast<unsigned char>(crc & 0xff));
        data.push_back(static_cast<unsigned char>((crc & 0xff00) >> 8));
        data.push_back(static_cast<unsigned char>((crc & 0xff0000) >> 16));
        data.push_back(static_cast<unsigned char>((crc & 0xff000000) >> 24));
    }
    data.push_back(0x80);
    crypto->initBuf();
    ByteVector encBuffer = crypto->desfireEncrypt(data, ByteVector(), false);
    buf.insert(buf.end(), encBuffer.begin(), encBuffer.end());
    transmit_plain(DFEV1_INS_SET_CONFIGURATION, buf);
}

ByteVector DESFireEV1ISO7816Commands::transmit(unsigned char cmd, const ByteVector &buf,
                                               unsigned char lc, bool forceLc)
{
    ByteVector CMAC;
    std::shared_ptr<DESFireCrypto> crypto = getDESFireChip()->getCrypto();

    if (crypto->d_auth_method != CM_LEGACY && cmd != DF_INS_ADDITIONAL_FRAME)
    {
        ByteVector apdu_command;
        apdu_command.push_back(cmd);
        apdu_command.insert(apdu_command.end(), buf.begin(), buf.end());
        crypto->desfire_cmac(apdu_command); // Ignore result
    }

    ByteVector r      = transmit_plain(cmd, buf, lc, forceLc);
    unsigned char err = r.back();
    if (crypto->d_auth_method != CM_LEGACY && cmd != DF_INS_ADDITIONAL_FRAME &&
        r.size() >= 10 && err != DF_INS_ADDITIONAL_FRAME)
    {
        ByteVector response = ByteVector(r.begin(), r.end() - 10);
        response.push_back(err);

        CMAC = crypto->desfire_cmac(response);

        EXCEPTION_ASSERT_WITH_LOG(ByteVector(r.end() - 10, r.end() - 2) == CMAC,
                                  LibLogicalAccessException, "Wrong CMAC.");

        r.erase(r.end() - 10, r.end() - 2);
    }

    return r;
}

ByteVector DESFireEV1ISO7816Commands::transmit_plain(unsigned char cmd, unsigned char lc)
{
    return transmit_plain(cmd, ByteVector(), lc, true);
}

ByteVector DESFireEV1ISO7816Commands::transmit_plain(unsigned char cmd,
                                                     const ByteVector &buf,
                                                     unsigned char lc, bool forceLc)
{
    return DESFireISO7816Commands::transmit(cmd, buf, lc, forceLc);
}

ByteVector DESFireEV1ISO7816Commands::transmit_nomacv(unsigned char cmd, unsigned char lc)
{
    return transmit_nomacv(cmd, ByteVector(), lc, true);
}

ByteVector DESFireEV1ISO7816Commands::transmit_nomacv(unsigned char cmd,
                                                      const ByteVector &buf,
                                                      unsigned char lc, bool forceLc)
{
    std::shared_ptr<DESFireCrypto> crypto = getDESFireChip()->getCrypto();
    if (crypto->d_auth_method != CM_LEGACY && cmd != DF_INS_ADDITIONAL_FRAME)
    {
        ByteVector apdu_command;
        apdu_command.push_back(cmd);
        apdu_command.insert(apdu_command.end(), buf.begin(), buf.end());
        crypto->desfire_cmac(apdu_command);
    }

    return DESFireISO7816Commands::transmit(cmd, buf, lc, forceLc);
}

MyDivInfo DESFireEV1ISO7816Commands::extract_iks_div_info(std::shared_ptr<Key> key,
                                                          uint8_t keyno)
{
    MyDivInfo ret{};

    auto diversification = key->getKeyDiversification();
    if (diversification && diversification->getType() == "NXPAV2")
    {
        auto crypto = getDESFireChip()->getCrypto();

        ByteVector diversify;
        if (key->getKeyDiversification())
            key->getKeyDiversification()->initDiversification(
                crypto->getIdentifier(), crypto->d_currentAid, key, keyno, diversify);

        ret.div_input = diversify;
        ret.div_type  = "NXPAV2";
    }

    return ret;
}

void DESFireEV1ISO7816Commands::iks_iso_authenticate(std::shared_ptr<DESFireKey> key,
                                                     bool isMasterCardKey, uint8_t keyno)
{
    assert(key->getKeyType() == DF_KEY_AES &&
           key->getKeyStorage()->getType() == KST_SERVER);
    std::shared_ptr<DESFireCrypto> crypto = getDESFireChip()->getCrypto();
    ByteVector RPICC1                     = iso_getChallenge(16); // 16 because aes
    auto kst = std::dynamic_pointer_cast<IKSStorage>(key->getKeyStorage());
    assert(kst);

    auto remote_crypto = LibraryManager::getInstance()->getRemoteCrypto();

    bool out_success;
    ByteVector out_random2, out_encrypted_cryptogram, out_auth_context_id;
    remote_crypto->iso_authenticate_step1(
        kst->getKeyIdentity(), RPICC1, extract_iks_div_info(key, keyno), out_success,
        out_random2, out_encrypted_cryptogram, out_auth_context_id);
    /*
        CMSG_DesfireISOAuth_Step1 step1_req;
        step1_req.set_key_uuid(
            std::dynamic_pointer_cast<IKSStorage>(key->getKeyStorage())->getKeyIdentity());
        step1_req.set_random_picc(std::string(RPICC1.begin(), RPICC1.end()));
        *step1_req.mutable_diversification() = extract_iks_div_info(key, keyno);

        SMSG_DesfireISOAuth_Step1 step1_resp =
       rpc_client.desfire_auth_iso_step1(step1_req);
    */

    /*iks::DesfireAuthCommand cmd;
    cmd.algo_ = iks::DESFIRE_AUTH_ALGO_AES;
    cmd.step_ = 1;
    cmd.key_idt_ =
        std::dynamic_pointer_cast<IKSStorage>(key->getKeyStorage())->getKeyIdentity();
    copy(RPICC1.begin(), RPICC1.end(), cmd.data_.begin());
    cmd.div_info_ = iks::KeyDivInfo::build(key, getChip()->getChipIdentifier(), keyno,
                                           crypto->d_currentAid);

    iks.send_command(cmd);
    auto resp = std::dynamic_pointer_cast<iks::DesfireAuthResponse>(iks.recv());
    EXCEPTION_ASSERT_WITH_LOG(resp, IKSException,
                              "Cannot retrieve proper response from server.");
                              */

    auto cryptogram = out_encrypted_cryptogram;
    iso_externalAuthenticate(DF_ALG_AES, isMasterCardKey, keyno, cryptogram);

    auto RPCD2 = ByteVector(out_random2.begin(), out_random2.end());
    cryptogram =
        iso_internalAuthenticate(DF_ALG_AES, isMasterCardKey, keyno, RPCD2, 2 * 16);

    ByteVector out_session_key, out_session_key_ref;
    remote_crypto->iso_authenticate_step2(kst->getKeyIdentity(), cryptogram,
                                          out_auth_context_id,
                                          extract_iks_div_info(key, keyno), out_success,
                                          out_session_key, out_session_key_ref);

    /*
    CMSG_DesfireAuth_Step2 step2_req;
    step2_req.set_key_uuid(
        std::dynamic_pointer_cast<IKSStorage>(key->getKeyStorage())->getKeyIdentity());
    step2_req.set_auth_context_id(step1_resp.auth_context_id());
    step2_req.set_picc_cryptogram(std::string(cryptogram.begin(), cryptogram.end()));
    *step2_req.mutable_diversification() = extract_iks_div_info(key, keyno);*/

    /*    cmd.step_     = 2;
        cmd.div_info_ = iks::KeyDivInfo::build(key, getChip()->getChipIdentifier(), keyno,
                                               crypto->d_currentAid);

        copy(cryptogram.begin(), cryptogram.end(), cmd.data_.begin());
        iks.send_command(cmd);
        resp = std::dynamic_pointer_cast<iks::DesfireAuthResponse>(iks.recv());
        EXCEPTION_ASSERT_WITH_LOG(resp, IKSException,
                                  "Cannot retrieve proper response from server.");
        EXCEPTION_ASSERT_WITH_LOG(resp->success_, IKSException,
                                  "Mutual authentication failure.");*/
    // SMSG_DesfireAuth_Step2 step2_resp = rpc_client.desfire_auth_iso_step2(step2_req);
    EXCEPTION_ASSERT_WITH_LOG(out_success, IKSException,
                              "Mutual authentication failure.");

    crypto->d_currentKeyNo = keyno;
    crypto->d_sessionKey.clear();
    crypto->d_auth_method = CM_ISO;

    // Session key from IKS. Either it is an IKS key reference, or directly
    // the key value.
    if (!out_session_key.empty())
    {
        // Raw key material. Use as is.
        crypto->d_sessionKey = ByteVector(out_session_key.begin(), out_session_key.end());
    }
    else
    {
        // We received a KeyReference. We need to create an IKSCryptoWrapper
        // for use by DESFireCrypto so CMAC calculation, data decryption and stuff
        // like that gets rerouted through IKS.
        crypto->iks_wrapper_ = std::make_unique<IKSCryptoWrapper>();
        crypto->iks_wrapper_->remote_key_name =
            std::string(out_session_key_ref.begin(), out_session_key_ref.end());
    }

    crypto->d_cipher.reset(new openssl::AESCipher());
    crypto->d_block_size = 16;
    crypto->d_mac_size   = 8;

    // common
    crypto->d_lastIV.clear();
    crypto->d_lastIV.resize(crypto->d_block_size, 0x00);

    onAuthenticated();
}

void DESFireEV1ISO7816Commands::selectApplication(unsigned int aid)
{
    DESFireISO7816Commands::selectApplication(aid);
}

void DESFireEV1ISO7816Commands::onAuthenticated()
{
    // If we don't have the read UID, we retrieve it
    if (!getDESFireChip()->hasRealUID())
    {
        getChip()->setChipIdentifier(getCardUID());
        getDESFireChip()->setHasRealUID(true);
    }
}

SignatureResult DESFireEV1ISO7816Commands::IKS_getLastReadSignature() const
{
    return handle_read_data_last_sig_;
}

void DESFireEV1ISO7816Commands::pkcs_iso_authenticate(
    std::shared_ptr<DESFireKey> currentKey, bool isMasterCardKey, unsigned char keyno)
{

    AESCryptoService aes_crypto;
    unsigned char le;
    std::shared_ptr<DESFireKey> key = std::make_shared<DESFireKey>(*currentKey);
    std::shared_ptr<openssl::SymmetricCipher> cipher;
    std::shared_ptr<DESFireCrypto> crypto = getDESFireChip()->getCrypto();

    auto diversify = getKeyInformations(key, keyno);

    ByteVector keydiv;
    ByteVector iv = {};
    crypto->getKey(key, diversify, keydiv);
    le = 16;
    cipher.reset(new openssl::AESCipher());

    ByteVector RPICC1 = iso_getChallenge(le);

    ByteVector RPCD1;
    RPCD1.resize(le);

    EXCEPTION_ASSERT_WITH_LOG(RAND_status() == 1, LibLogicalAccessException,
                              "Insufficient enthropy source");
    if (RAND_bytes(&RPCD1[0], static_cast<int>(RPCD1.size())) != 1)
    {
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "Cannot retrieve cryptographically strong bytes");
    }
    ByteVector makecrypt1;
    makecrypt1.insert(makecrypt1.end(), RPCD1.begin(), RPCD1.end());
    makecrypt1.insert(makecrypt1.end(), RPICC1.begin(), RPICC1.end());
    ByteVector cryptogram = aes_crypto.aes_encrypt(makecrypt1, {}, currentKey);
    iv                    = ByteVector(cryptogram.end() - 16, cryptogram.end());
    iso_externalAuthenticate(DF_ALG_AES, isMasterCardKey, keyno, cryptogram);

    ByteVector RPCD2;
    RPCD2.resize(le);

    EXCEPTION_ASSERT_WITH_LOG(RAND_status() == 1, LibLogicalAccessException,
                              "Insufficient enthropy source");
    if (RAND_bytes(&RPCD2[0], static_cast<int>(RPCD2.size())) != 1)
    {
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "Cannot retrieve cryptographically strong bytes");
    }
    cryptogram =
        iso_internalAuthenticate(DF_ALG_AES, isMasterCardKey, keyno, RPCD2, 2 * le);

    if (cryptogram.size() < 1)
        THROW_EXCEPTION_WITH_LOG(CardException, "iso_authenticate wrong internal data.");

    ByteVector response = aes_crypto.aes_decrypt(cryptogram, iv, currentKey);

    ByteVector RPICC2 = ByteVector(response.begin(), response.begin() + le);
    ByteVector RPCD2a = ByteVector(response.begin() + le, response.end());

    EXCEPTION_ASSERT_WITH_LOG(RPCD2 == RPCD2a, CardException,
                              "Integrity error : host part of mutual authentication");

    crypto->d_currentKeyNo = keyno;

    crypto->d_sessionKey.clear();
    crypto->d_auth_method = CM_ISO;
    crypto->d_sessionKey.insert(crypto->d_sessionKey.end(), RPCD1.begin(),
                                RPCD1.begin() + 4);
    crypto->d_sessionKey.insert(crypto->d_sessionKey.end(), RPICC2.begin(),
                                RPICC2.begin() + 4);
    crypto->d_sessionKey.insert(crypto->d_sessionKey.end(), RPCD1.begin() + 12,
                                RPCD1.begin() + 16);
    crypto->d_sessionKey.insert(crypto->d_sessionKey.end(), RPICC2.begin() + 12,
                                RPICC2.begin() + 16);

    crypto->d_cipher.reset(new openssl::AESCipher());
    crypto->d_authkey    = keydiv;
    crypto->d_block_size = 16;
    crypto->d_mac_size   = 8;

    crypto->d_lastIV.clear();
    crypto->d_lastIV.resize(crypto->d_block_size, 0x00);

    onAuthenticated();
}
}
