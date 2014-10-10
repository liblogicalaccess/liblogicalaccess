/**
 * \file desfireev1iso7816commands.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief DESFireEV1 ISO7816 commands.
 */

#include "../commands/desfireev1iso7816commands.hpp"
#include "desfirechip.hpp"
#include <openssl/rand.h>
#include "logicalaccess/logs.hpp"
#include "logicalaccess/crypto/aes_cipher.hpp"
#include "logicalaccess/crypto/aes_symmetric_key.hpp"
#include "logicalaccess/crypto/aes_initialization_vector.hpp"
#include "logicalaccess/crypto/des_cipher.hpp"
#include "logicalaccess/crypto/des_symmetric_key.hpp"
#include "logicalaccess/crypto/des_initialization_vector.hpp"
#include "desfireev1location.hpp"
#include "logicalaccess/cards/samkeystorage.hpp"
#include "samcommands.hpp"
#include "nxpav2keydiversification.hpp"

namespace logicalaccess
{
    DESFireEV1ISO7816Commands::DESFireEV1ISO7816Commands()
        : DESFireISO7816Commands()
    {
    }

    DESFireEV1ISO7816Commands::~DESFireEV1ISO7816Commands()
    {
    }

    unsigned int DESFireEV1ISO7816Commands::getFreeMem()
    {
        unsigned int freemem = 0;

        std::vector<unsigned char> r = transmit(DFEV1_INS_FREE_MEM);
        r.resize(r.size() - 2);

        if (r.size() == 3)
        {
            freemem = (r[0] << 16) |
                (r[1] << 8) |
                r[2];
        }

        return freemem;
    }

    vector<DFName> DESFireEV1ISO7816Commands::getDFNames()
    {
        vector<DFName> dfnames;
        unsigned char err;
        unsigned char cmd = DFEV1_INS_GET_DF_NAMES;

        do
        {
            std::vector<unsigned char> r = transmit(cmd);
            err = r[0];
            r.resize(r.size() - 2);
            cmd = DF_INS_ADDITIONAL_FRAME;

            DFName dfname;
            memset(&dfname, 0x00, sizeof(dfname));
            dfname.AID = DESFireLocation::convertAidToUInt(r);
            dfname.FID = static_cast<unsigned short>((r[4] << 8) | r[3]);
            memcpy(dfname.DF_Name, &r[5], r.size() - 5);
            dfnames.push_back(dfname);
        } while (err == DF_INS_ADDITIONAL_FRAME);

        return dfnames;
    }

    vector<unsigned short> DESFireEV1ISO7816Commands::getISOFileIDs()
    {
        vector<unsigned short> fileids;
        unsigned char err;
        unsigned char cmd = DFEV1_INS_GET_ISO_FILE_IDS;
        std::vector<unsigned char> fullr;

        do
        {
            std::vector<unsigned char> r = transmit(cmd);
            err = r[0];
            r.resize(r.size() - 2);
            cmd = DF_INS_ADDITIONAL_FRAME;

            fullr.insert(r.end(), r.begin(), r.end());
        } while (err == DF_INS_ADDITIONAL_FRAME);

        for (unsigned int i = 0; i < fullr.size(); i += 2)
        {
            unsigned short fid = static_cast<unsigned short>((fullr[i] << 8) | fullr[i + 1]);
            fileids.push_back(fid);
        }

        return fileids;
    }

    void DESFireEV1ISO7816Commands::createApplication(unsigned int aid, DESFireKeySettings settings, unsigned char maxNbKeys, DESFireKeyType cryptoMethod, FidSupport fidSupported, unsigned short isoFID, std::vector<unsigned char> isoDFName)
    {
        std::vector<unsigned char> command;

        DESFireLocation::convertUIntToAid(aid, command);
        command.push_back(static_cast<unsigned char>(settings));
        command.push_back(static_cast<unsigned char>((maxNbKeys & 0x0f) | fidSupported | cryptoMethod));

        if (isoFID != 0x00)
        {
            command.push_back(static_cast<unsigned char>(isoFID & 0xff));
            command.push_back(static_cast<unsigned char>((isoFID & 0xff00) >> 8));
        }
        if (isoDFName.size() > 0)
        {
            command.insert(command.end(), isoDFName.begin(), isoDFName.end());
        }

        transmit(DF_INS_CREATE_APPLICATION, command);
        d_crypto->createApplication(aid, maxNbKeys, cryptoMethod);
    }

    void DESFireEV1ISO7816Commands::getKeySettings(DESFireKeySettings& settings, unsigned char& maxNbKeys, DESFireKeyType& keyType)
    {
        std::vector<unsigned char> r = transmit(DF_INS_GET_KEY_SETTINGS);
        r.resize(r.size() - 2);

        settings = static_cast<DESFireKeySettings>(r[0]);
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

    std::vector<unsigned char> DESFireEV1ISO7816Commands::getCardUID()
    {
        std::vector<unsigned char> result = transmit_nomacv(DFEV1_INS_GET_CARD_UID);
        result.resize(result.size() - 2);

        return d_crypto->desfire_iso_decrypt(result, 7);
    }

    void DESFireEV1ISO7816Commands::createStdDataFile(unsigned char fileno, EncryptionMode comSettings, DESFireAccessRights accessRights, unsigned int fileSize, unsigned short isoFID)
    {
        std::vector<unsigned char> command;
        short ar = AccessRightsInMemory(accessRights);

        command.push_back(static_cast<unsigned char>(fileno));
        if (isoFID != 0)
        {
            command.push_back(static_cast<unsigned char>(isoFID & 0xff));
            command.push_back(static_cast<unsigned char>(static_cast<unsigned short>(isoFID & 0xff00) >> 8));
        }
        command.push_back(static_cast<unsigned char>(comSettings));
        BufferHelper::setUShort(command, ar);
        command.push_back(static_cast<unsigned char>(fileSize & 0xff));
        command.push_back(static_cast<unsigned char>(static_cast<unsigned short>(fileSize & 0xff00) >> 8));
        command.push_back(static_cast<unsigned char>(static_cast<unsigned int>(fileSize & 0xff0000) >> 16));

        DESFireISO7816Commands::transmit(DF_INS_CREATE_STD_DATA_FILE, command);
    }

    void DESFireEV1ISO7816Commands::createBackupFile(unsigned char fileno, EncryptionMode comSettings, DESFireAccessRights accessRights, unsigned int fileSize, unsigned short isoFID)
    {
        std::vector<unsigned char> command;
        short ar = AccessRightsInMemory(accessRights);

        command.push_back(static_cast<unsigned char>(fileno));
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

        DESFireISO7816Commands::transmit(DF_INS_CREATE_BACKUP_DATA_FILE, command);
    }

    void DESFireEV1ISO7816Commands::createLinearRecordFile(unsigned char fileno, EncryptionMode comSettings, DESFireAccessRights accessRights, unsigned int fileSize, unsigned int maxNumberOfRecords, unsigned short isoFID)
    {
        std::vector<unsigned char> command;
        short ar = AccessRightsInMemory(accessRights);

        command.push_back(static_cast<unsigned char>(fileno));
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

        DESFireISO7816Commands::transmit(DF_INS_CREATE_LINEAR_RECORD_FILE, command);
    }

    void DESFireEV1ISO7816Commands::createCyclicRecordFile(unsigned char fileno, EncryptionMode comSettings, DESFireAccessRights accessRights, unsigned int fileSize, unsigned int maxNumberOfRecords, unsigned short isoFID)
    {
        std::vector<unsigned char> command;
        short ar = AccessRightsInMemory(accessRights);

        command.push_back(static_cast<unsigned char>(fileno));
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

        DESFireISO7816Commands::transmit(DF_INS_CREATE_CYCLIC_RECORD_FILE, command[0]);;
    }

    void DESFireEV1ISO7816Commands::iso_selectFile(unsigned short fid)
    {
        std::vector<unsigned char> data;
        data.push_back(static_cast<unsigned char>(static_cast<unsigned short>(fid & 0xff00) >> 8));
        data.push_back(static_cast<unsigned char>(fid & 0xff));

        DESFireISO7816Commands::getISO7816ReaderCardAdapter()->sendAPDUCommand(DFEV1_CLA_ISO_COMPATIBLE, ISO7816_INS_SELECT_FILE, 0x00, 0x0C, static_cast<unsigned char>(data.size()), data);
    }

    std::vector<unsigned char> DESFireEV1ISO7816Commands::iso_readRecords(unsigned short fid, unsigned char start_record, DESFireRecords record_number)
    {
        std::vector<unsigned char> result;

        unsigned char p1 = start_record;
        unsigned char p2 = record_number & 0x0f;
        if (fid != 0)
        {
            p2 += static_cast<unsigned char>((fid & 0xff) << 3);
        }

        result = DESFireISO7816Commands::getISO7816ReaderCardAdapter()->sendAPDUCommand(DFEV1_CLA_ISO_COMPATIBLE, ISO7816_INS_READ_RECORDS, p1, p2, 0x00);

        return std::vector<unsigned char>(result.begin(), result.end() - 2);
    }

    void DESFireEV1ISO7816Commands::iso_appendrecord(const std::vector<unsigned char>& data, unsigned short fid)
    {
        unsigned char p1 = 0x00;
        unsigned char p2 = 0x00;
        if (fid != 0)
        {
            p2 += static_cast<unsigned char>((fid & 0xff) << 3);
        }

        DESFireISO7816Commands::getISO7816ReaderCardAdapter()->sendAPDUCommand(DFEV1_CLA_ISO_COMPATIBLE, ISO7816_INS_APPEND_RECORD, p1, p2, static_cast<unsigned char>(data.size()), data);
    }

    std::vector<unsigned char> DESFireEV1ISO7816Commands::iso_getChallenge(unsigned int length)
    {
        std::vector<unsigned char> result;

        result = DESFireISO7816Commands::getISO7816ReaderCardAdapter()->sendAPDUCommand(DFEV1_CLA_ISO_COMPATIBLE, ISO7816_INS_GET_CHALLENGE, 0x00, 0x00, static_cast<unsigned char>(length));

        if (result[result.size() - 2] == 0x90 && result[result.size() - 1] == 0x00)
        {
            return std::vector<unsigned char>(result.begin(), result.end() - 2);
        }
        return result;
    }

    void DESFireEV1ISO7816Commands::iso_externalAuthenticate(DESFireISOAlgorithm algorithm, bool isMasterCardKey, unsigned char keyno, const std::vector<unsigned char>& data)
    {
        unsigned char p1 = static_cast<unsigned char>(algorithm);
        unsigned char p2 = 0x00;
        if (!isMasterCardKey)
        {
            p2 = static_cast<unsigned char>(0x80 + (keyno & 0x0F));
        }

        DESFireISO7816Commands::getISO7816ReaderCardAdapter()->sendAPDUCommand(DFEV1_CLA_ISO_COMPATIBLE, ISO7816_INS_EXTERNAL_AUTHENTICATE, p1, p2, static_cast<unsigned char>(data.size()), data);
    }

    std::vector<unsigned char> DESFireEV1ISO7816Commands::iso_internalAuthenticate(DESFireISOAlgorithm algorithm, bool isMasterCardKey, unsigned char keyno, const std::vector<unsigned char>& RPCD2, unsigned int length)
    {
        std::vector<unsigned char> result;

        unsigned char p1 = static_cast<unsigned char>(algorithm);
        unsigned char p2 = 0x00;
        if (!isMasterCardKey)
        {
            p2 = static_cast<unsigned char>(0x80 + (keyno & 0x0F));
        }

        result = DESFireISO7816Commands::getISO7816ReaderCardAdapter()->sendAPDUCommand(DFEV1_CLA_ISO_COMPATIBLE, ISO7816_INS_INTERNAL_AUTHENTICATE, p1, p2, static_cast<unsigned char>(RPCD2.size()), RPCD2, static_cast<unsigned char>(length));

        if (result[result.size() - 2] == 0x90 && result[result.size() - 1] == 0x00)
        {
            return std::vector<unsigned char>(result.begin(), result.end() - 2);
        }
        return result;
    }

    void DESFireEV1ISO7816Commands::authenticate(unsigned char keyno, boost::shared_ptr<DESFireKey> key)
    {
        if (!key) {
            key = boost::dynamic_pointer_cast<DESFireProfile>(getChip()->getProfile())->getDefaultKey(DF_KEY_DES);
        }

        // Get the appropriate authentification method and algorithm according to the key type (for 3DES we use legacy method instead of ISO).

        if (boost::dynamic_pointer_cast<SAMKeyStorage>(key->getKeyStorage()) && !getSAMChip())
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "SAMKeyStorage set on the key but not SAM reader has been set.");

        if (boost::dynamic_pointer_cast<SAMKeyStorage>(key->getKeyStorage()) && key->getKeyType() != DF_KEY_DES)
        {
            if (key->getKeyType() == DF_KEY_3K3DES)
                sam_iso_authenticate(key, DF_ALG_3K3DES, (d_crypto->d_currentAid == 0 && keyno == 0), keyno);
            else
                sam_iso_authenticate(key, DF_ALG_AES, (d_crypto->d_currentAid == 0 && keyno == 0), keyno);
        }
        else
        {
            switch (key->getKeyType())
            {
            case DF_KEY_DES:
                DESFireISO7816Commands::authenticate(keyno, key);
                break;

            case DF_KEY_3K3DES:
                iso_authenticate(key, DF_ALG_3K3DES, (d_crypto->d_currentAid == 0 && keyno == 0), keyno);
                break;

            case DF_KEY_AES:
                iso_authenticate(key, DF_ALG_AES, (d_crypto->d_currentAid == 0 && keyno == 0), keyno);
                break;
            }
        }
    }

    void DESFireEV1ISO7816Commands::sam_iso_authenticate(boost::shared_ptr<DESFireKey> key, DESFireISOAlgorithm algorithm, bool isMasterCardKey, unsigned char keyno)
    {
        unsigned char p1 = 0x00; //0x02 if selectApplication has been proceed
        std::vector<unsigned char> diversify;

        if (key->getKeyDiversification())
        {
            key->getKeyDiversification()->initDiversification(d_crypto->getIdentifier(), d_crypto->d_currentAid, key, keyno, diversify);
        }

        std::vector<unsigned char> apduresult;

        std::vector<unsigned char> RPICC1 = iso_getChallenge(16);

        std::vector<unsigned char> data(2 + RPICC1.size());

        if (!boost::dynamic_pointer_cast<SAMKeyStorage>(key->getKeyStorage()))
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "DESFireKey need a SAMKeyStorage to proceed a SAM ISO Authenticate.");

        data[0] = boost::dynamic_pointer_cast<SAMKeyStorage>(key->getKeyStorage())->getKeySlot();
        data[1] = key->getKeyVersion();
        memcpy(&data[0] + 2, &RPICC1[0], RPICC1.size());

        if (boost::dynamic_pointer_cast<NXPKeyDiversification>(key->getKeyDiversification()))
        {
            p1 |= 0x01;
            if (boost::dynamic_pointer_cast<NXPAV2KeyDiversification>(key->getKeyDiversification()))
                p1 |= 0x10;
            data.insert(data.end(), diversify.begin(), diversify.end());
        }

        unsigned char cmdp1[] = { 0x80, 0x8e, p1, 0x00, (unsigned char)(data.size()), 0x00 };
        std::vector<unsigned char> cmd_vector(cmdp1, cmdp1 + 6);
        cmd_vector.insert(cmd_vector.end() - 1, data.begin(), data.end());

        if (getSAMChip()->getCardType() == "SAM_AV1")
            apduresult = boost::dynamic_pointer_cast<SAMCommands<KeyEntryAV1Information, SETAV1>>(getSAMChip()->getCommands())->transmit(cmd_vector);
        else if (getSAMChip()->getCardType() == "SAM_AV2")
            apduresult = boost::dynamic_pointer_cast<SAMCommands<KeyEntryAV2Information, SETAV2>>(getSAMChip()->getCommands())->transmit(cmd_vector, true, false);
        if (apduresult.size() <= 2 && apduresult[apduresult.size() - 2] != 0x90 && apduresult[apduresult.size() - 2] != 0xaf)
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "sam_iso_authenticate P1 failed.");

        std::vector<unsigned char> encRPCD1RPICC1(apduresult.begin(), apduresult.end() - 2 - 16);
        std::vector<unsigned char> RPCD2(apduresult.end() - 16 - 2, apduresult.end() - 2);

        std::vector<unsigned char> encRPICC2RPCD2a;
        try
        {
            iso_externalAuthenticate(algorithm, isMasterCardKey, keyno, encRPCD1RPICC1);
            encRPICC2RPCD2a = iso_internalAuthenticate(algorithm, isMasterCardKey, keyno, RPCD2, 2 * 16);
        }
        catch (...)
        {
            std::exception_ptr eptr = std::current_exception();
            // Cancel SAM authentication with dummy command, but ignore return
            try
            {
                unsigned char resetcmd[] = { 0x80, 0xaf, 0x00, 0x00, 0x00 };
                cmd_vector.assign(resetcmd, resetcmd + 5);
                if (getSAMChip()->getCardType() == "SAM_AV1")
                    apduresult = boost::dynamic_pointer_cast<SAMCommands<KeyEntryAV1Information, SETAV1>>(getSAMChip()->getCommands())->transmit(cmd_vector);
                else if (getSAMChip()->getCardType() == "SAM_AV2")
                    apduresult = boost::dynamic_pointer_cast<SAMCommands<KeyEntryAV2Information, SETAV2>>(getSAMChip()->getCommands())->transmit(cmd_vector);
            }
            catch (std::exception&){}

            std::rethrow_exception(eptr);
        }

        if (encRPICC2RPCD2a.size() < 1)
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "sam_iso_authenticate wrong internal data.");

        unsigned char cmdp2[] = { 0x80, 0x8e, 0x00, 0x00, (unsigned char)(encRPICC2RPCD2a.size()) };
        cmd_vector.assign(cmdp2, cmdp2 + 5);
        cmd_vector.insert(cmd_vector.end(), encRPICC2RPCD2a.begin(), encRPICC2RPCD2a.end());
        if (getSAMChip()->getCardType() == "SAM_AV1")
            apduresult = boost::dynamic_pointer_cast<SAMCommands<KeyEntryAV1Information, SETAV1>>(getSAMChip()->getCommands())->transmit(cmd_vector);
        else if (getSAMChip()->getCardType() == "SAM_AV2")
            apduresult = boost::dynamic_pointer_cast<SAMCommands<KeyEntryAV2Information, SETAV2>>(getSAMChip()->getCommands())->transmit(cmd_vector);
        if (apduresult.size() <= 2 && apduresult[apduresult.size() - 2] != 0x90 && apduresult[apduresult.size() - 2] != 0x00)
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "sam_iso_authenticate P2 failed.");

        d_crypto->d_currentKeyNo = keyno;
        if (getSAMChip()->getCardType() == "SAM_AV1")
            d_crypto->d_sessionKey = boost::dynamic_pointer_cast<SAMCommands<KeyEntryAV1Information, SETAV1>>(getSAMChip()->getCommands())->dumpSessionKey();
        else if (getSAMChip()->getCardType() == "SAM_AV2")
            d_crypto->d_sessionKey = boost::dynamic_pointer_cast<SAMCommands<KeyEntryAV2Information, SETAV2>>(getSAMChip()->getCommands())->dumpSessionKey();
        d_crypto->d_auth_method = CM_ISO;

        LOG(LogLevel::INFOS) << "Session key length: " << d_crypto->d_sessionKey.size();

        if (key->getKeyType() == DF_KEY_3K3DES || key->getKeyType() == DF_KEY_DES)
        {
            d_crypto->d_cipher.reset(new openssl::DESCipher());
            d_crypto->d_block_size = 8;
            d_crypto->d_mac_size = 8;
        }
        else
        {
            d_crypto->d_cipher.reset(new openssl::AESCipher());
            d_crypto->d_block_size = 16;
            d_crypto->d_mac_size = 8;
        }
        d_crypto->d_lastIV.clear();
        d_crypto->d_lastIV.resize(d_crypto->d_block_size, 0x00);
    }

    void DESFireEV1ISO7816Commands::iso_authenticate(boost::shared_ptr<DESFireKey> key, DESFireISOAlgorithm algorithm, bool isMasterCardKey, unsigned char keyno)
    {
        unsigned char le;
        boost::shared_ptr<openssl::SymmetricCipher> cipher;
        std::vector<unsigned char> diversify;

        if (key->getKeyDiversification())
        {
            key->getKeyDiversification()->initDiversification(d_crypto->getIdentifier(), d_crypto->d_currentAid, key, keyno, diversify);
        }
        std::vector<unsigned char> keydiv;
        d_crypto->getKey(key, diversify, keydiv);

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
            cipher.reset(new openssl::AESCipher());
        }

        boost::shared_ptr<openssl::SymmetricKey> isokey;
        boost::shared_ptr<openssl::InitializationVector> iv;
        if (boost::dynamic_pointer_cast<openssl::AESCipher>(cipher))
        {
            isokey.reset(new openssl::AESSymmetricKey(openssl::AESSymmetricKey::createFromData(keydiv)));
            iv.reset(new openssl::AESInitializationVector(openssl::AESInitializationVector::createNull()));
        }
        else
        {
            isokey.reset(new openssl::DESSymmetricKey(openssl::DESSymmetricKey::createFromData(keydiv)));
            iv.reset(new openssl::DESInitializationVector(openssl::DESInitializationVector::createNull()));
        }

        std::vector<unsigned char> RPICC1 = iso_getChallenge(le);

        std::vector<unsigned char> RPCD1;
        RPCD1.resize(le);

        EXCEPTION_ASSERT_WITH_LOG(RAND_status() == 1, LibLogicalAccessException, "Insufficient enthropy source");
        if (RAND_bytes(&RPCD1[0], static_cast<int>(RPCD1.size())) != 1)
        {
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Cannot retrieve cryptographically strong bytes");
        }
        std::vector<unsigned char> makecrypt1;
        makecrypt1.insert(makecrypt1.end(), RPCD1.begin(), RPCD1.end());
        makecrypt1.insert(makecrypt1.end(), RPICC1.begin(), RPICC1.end());
        std::vector<unsigned char> cryptogram;
        cipher->cipher(makecrypt1, cryptogram, *isokey.get(), *iv.get(), false);
        if (boost::dynamic_pointer_cast<openssl::AESCipher>(cipher))
        {
            iv.reset(new openssl::AESInitializationVector(openssl::AESInitializationVector::createFromData(std::vector<unsigned char>(cryptogram.end() - iv->data().size(), cryptogram.end()))));
        }
        else
        {
            iv.reset(new openssl::DESInitializationVector(openssl::DESInitializationVector::createFromData(std::vector<unsigned char>(cryptogram.end() - iv->data().size(), cryptogram.end()))));
        }
        iso_externalAuthenticate(algorithm, isMasterCardKey, keyno, cryptogram);

        std::vector<unsigned char> RPCD2;
        RPCD2.resize(le);

        EXCEPTION_ASSERT_WITH_LOG(RAND_status() == 1, LibLogicalAccessException, "Insufficient enthropy source");
        if (RAND_bytes(&RPCD2[0], static_cast<int>(RPCD2.size())) != 1)
        {
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Cannot retrieve cryptographically strong bytes");
        }
        cryptogram = iso_internalAuthenticate(algorithm, isMasterCardKey, keyno, RPCD2, 2 * le);

        if (cryptogram.size() < 1)
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "iso_authenticate wrong internal data.");

        std::vector<unsigned char> response;
        cipher->decipher(cryptogram, response, *isokey.get(), *iv.get(), false);

        std::vector<unsigned char> RPICC2 = std::vector<unsigned char>(response.begin(), response.begin() + le);
        std::vector<unsigned char> RPCD2a = std::vector<unsigned char>(response.begin() + le, response.end());

        EXCEPTION_ASSERT_WITH_LOG(RPCD2 == RPCD2a, LibLogicalAccessException, "Integrity error : host part of mutual authentication");

        d_crypto->d_currentKeyNo = keyno;

        d_crypto->d_sessionKey.clear();
        d_crypto->d_auth_method = CM_ISO;
        if (algorithm == DF_ALG_2K3DES)
        {
            if (std::vector<unsigned char>(keydiv.begin(), keydiv.begin() + 8) == std::vector<unsigned char>(keydiv.begin() + 8, keydiv.begin() + 8 + 8))
            {
                d_crypto->d_sessionKey.insert(d_crypto->d_sessionKey.end(), RPCD1.begin(), RPCD1.begin() + 4);
                d_crypto->d_sessionKey.insert(d_crypto->d_sessionKey.end(), RPICC2.begin(), RPCD1.begin() + 4);
                d_crypto->d_sessionKey.insert(d_crypto->d_sessionKey.end(), RPCD1.begin(), RPCD1.begin() + 4);
                d_crypto->d_sessionKey.insert(d_crypto->d_sessionKey.end(), RPICC2.begin(), RPCD1.begin() + 4);
            }
            else
            {
                d_crypto->d_sessionKey.insert(d_crypto->d_sessionKey.end(), RPCD1.begin(), RPCD1.begin() + 4);
                d_crypto->d_sessionKey.insert(d_crypto->d_sessionKey.end(), RPICC2.begin(), RPCD1.begin() + 4);
                d_crypto->d_sessionKey.insert(d_crypto->d_sessionKey.end(), RPCD1.begin() + 4, RPCD1.begin() + 8);
                d_crypto->d_sessionKey.insert(d_crypto->d_sessionKey.end(), RPICC2.begin() + 4, RPCD1.begin() + 8);
            }

            d_crypto->d_cipher.reset(new openssl::DESCipher());
            d_crypto->d_authkey = keydiv;
            d_crypto->d_block_size = 8;
            d_crypto->d_mac_size = 8;
        }
        else if (algorithm == DF_ALG_3K3DES)
        {
            d_crypto->d_sessionKey.insert(d_crypto->d_sessionKey.end(), RPCD1.begin(), RPCD1.begin() + 4);
            d_crypto->d_sessionKey.insert(d_crypto->d_sessionKey.end(), RPICC2.begin(), RPICC2.begin() + 4);
            d_crypto->d_sessionKey.insert(d_crypto->d_sessionKey.end(), RPCD1.begin() + 6, RPCD1.begin() + 10);
            d_crypto->d_sessionKey.insert(d_crypto->d_sessionKey.end(), RPICC2.begin() + 6, RPICC2.begin() + 10);
            d_crypto->d_sessionKey.insert(d_crypto->d_sessionKey.end(), RPCD1.begin() + 12, RPCD1.begin() + 16);
            d_crypto->d_sessionKey.insert(d_crypto->d_sessionKey.end(), RPICC2.begin() + 12, RPICC2.begin() + 16);

            d_crypto->d_cipher.reset(new openssl::DESCipher());
            d_crypto->d_authkey = keydiv;
            d_crypto->d_block_size = 8;
            d_crypto->d_mac_size = 8;
        }
        else
        {
            d_crypto->d_sessionKey.insert(d_crypto->d_sessionKey.end(), RPCD1.begin(), RPCD1.begin() + 4);
            d_crypto->d_sessionKey.insert(d_crypto->d_sessionKey.end(), RPICC2.begin(), RPICC2.begin() + 4);
            d_crypto->d_sessionKey.insert(d_crypto->d_sessionKey.end(), RPCD1.begin() + 12, RPCD1.begin() + 16);
            d_crypto->d_sessionKey.insert(d_crypto->d_sessionKey.end(), RPICC2.begin() + 12, RPICC2.begin() + 16);

            d_crypto->d_cipher.reset(new openssl::AESCipher());
            d_crypto->d_authkey = keydiv;
            d_crypto->d_block_size = 16;
            d_crypto->d_mac_size = 8;
        }
        d_crypto->d_lastIV.clear();
        d_crypto->d_lastIV.resize(d_crypto->d_block_size, 0x00);
    }

    void DESFireEV1ISO7816Commands::authenticateISO(unsigned char keyno, DESFireISOAlgorithm algorithm)
    {
        d_crypto->d_auth_method = CM_LEGACY; // To prevent CMAC checking

        unsigned char random_len;
        if (algorithm == DF_ALG_2K3DES)
        {
            random_len = 8;
        }
        else
        {
            random_len = 16;
        }

        std::vector<unsigned char> data;
        data.push_back(keyno);

        std::vector<unsigned char> diversify;
        boost::shared_ptr<DESFireKey> currentkey = boost::dynamic_pointer_cast<DESFireProfile>(getChip()->getProfile())->getKey(d_crypto->d_currentAid, keyno);
        if (currentkey->getKeyDiversification())
        {
            currentkey->getKeyDiversification()->initDiversification(d_crypto->getIdentifier(), d_crypto->d_currentAid, currentkey, keyno, diversify);
        }

        std::vector<unsigned char> encRndB = DESFireISO7816Commands::transmit(DFEV1_INS_AUTHENTICATE_ISO, data);
        unsigned char err = encRndB.back();
        encRndB.resize(encRndB.size() - 2);
        EXCEPTION_ASSERT_WITH_LOG(err == DF_INS_ADDITIONAL_FRAME, LibLogicalAccessException, "No additional frame for ISO authentication.");

        std::vector<unsigned char> response = d_crypto->iso_authenticate_PICC1(keyno, diversify, encRndB, random_len);
        std::vector<unsigned char> encRndA1 = DESFireISO7816Commands::transmit(DF_INS_ADDITIONAL_FRAME, response);
        encRndA1.resize(encRndA1.size() - 2);

        d_crypto->iso_authenticate_PICC2(keyno, encRndA1, random_len);
    }

    void DESFireEV1ISO7816Commands::authenticateAES(unsigned char keyno)
    {
        d_crypto->d_auth_method = CM_LEGACY; // To prevent CMAC checking

        std::vector<unsigned char> data;
        data.push_back(keyno);

        std::vector<unsigned char> diversify;
        boost::shared_ptr<DESFireKey> currentkey = boost::dynamic_pointer_cast<DESFireProfile>(getChip()->getProfile())->getKey(d_crypto->d_currentAid, keyno);
        if (currentkey->getKeyDiversification())
        {
            currentkey->getKeyDiversification()->initDiversification(d_crypto->getIdentifier(), d_crypto->d_currentAid, currentkey, keyno, diversify);
        }

        std::vector<unsigned char> encRndB = DESFireISO7816Commands::transmit(DFEV1_INS_AUTHENTICATE_AES, data);
        unsigned char err = encRndB.back();
        encRndB.resize(encRndB.size() - 2);
        EXCEPTION_ASSERT_WITH_LOG(err == DF_INS_ADDITIONAL_FRAME, LibLogicalAccessException, "No additional frame for ISO authentication.");

        std::vector<unsigned char> response = d_crypto->aes_authenticate_PICC1(keyno, diversify, encRndB);
        std::vector<unsigned char> encRndA1 = DESFireISO7816Commands::transmit(DF_INS_ADDITIONAL_FRAME, response);
        encRndA1.resize(encRndA1.size() - 2);

        d_crypto->aes_authenticate_PICC2(keyno, encRndA1);
    }

    std::vector<unsigned char> DESFireEV1ISO7816Commands::handleReadData(unsigned char err, const std::vector<unsigned char>& firstMsg, unsigned int length, EncryptionMode mode)
    {
        std::vector<unsigned char> ret, data;

        if ((err == DF_INS_ADDITIONAL_FRAME || err == 0x00))
        {
            if (mode == CM_ENCRYPT)
            {
                d_crypto->decipherData1(length, firstMsg);
            }
            else
            {
                ret = data = firstMsg;
                if (mode == CM_MAC)
                {
                    d_crypto->initBuf(length + 4);
                }
            }
        }

        while (err == DF_INS_ADDITIONAL_FRAME)
        {
            data = transmit_plain(DF_INS_ADDITIONAL_FRAME);
            err = data.back();
            data.resize(data.size() - 2);
            if (mode == CM_ENCRYPT)
            {
                d_crypto->decipherData2(data);
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
            if (d_crypto->d_auth_method != CM_LEGACY)
            {
                if (!d_crypto->verifyMAC(true, ret))
                {
                    THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "MAC data doesn't match.");
                }
                ret.resize(ret.size() - d_crypto->d_mac_size);
            }
        }
            break;

        case CM_MAC:
        {
            if (!d_crypto->verifyMAC(true, ret))
            {
                THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "MAC data doesn't match.");
            }
            ret.resize(ret.size() - d_crypto->d_mac_size);
        }
            break;
        case CM_ENCRYPT:
        {
            ret = d_crypto->decipherData(length);
        }
            break;
        case CM_UNKNOWN:
        {
        }
            break;
        }

        return ret;
    }

    void DESFireEV1ISO7816Commands::handleWriteData(unsigned char cmd, unsigned char* parameters, unsigned int paramLength, const std::vector<unsigned char>& data, EncryptionMode mode)
    {
        std::vector<unsigned char> edata, command;

        d_crypto->initBuf(data.size());
        size_t DESFIRE_EV1_CLEAR_DATA_LENGTH_CHUNK = (DESFIRE_CLEAR_DATA_LENGTH_CHUNK - 8);

        if (data.size() <= DESFIRE_EV1_CLEAR_DATA_LENGTH_CHUNK)
        {
            switch (mode)
            {
            case CM_PLAIN:
            {
                edata = data;
                if (d_crypto->d_auth_method != CM_LEGACY) // CMAC needs to be recalculated
                {
                    std::vector<unsigned char> apdu_command;
                    apdu_command.push_back(cmd);
                    if (parameters != NULL)
                    {
                        apdu_command.insert(apdu_command.end(), parameters, parameters + paramLength);
                    }
                    if (data.size() > 0)
                    {
                        apdu_command.insert(apdu_command.end(), data.begin(), data.end());
                    }
                    d_crypto->desfire_cmac(apdu_command);
                }
            }
                break;

            case CM_MAC:
            {
                std::vector<unsigned char> mac;
                if (d_crypto->d_auth_method == CM_LEGACY)
                {
                    mac = d_crypto->generateMAC(data);
                }
                else
                {
                    std::vector<unsigned char> gdata;
                    gdata.push_back(cmd);
                    if (parameters != NULL)
                    {
                        gdata.insert(gdata.end(), parameters, parameters + paramLength);
                    }
                    if (data.size() > 0)
                    {
                        gdata.insert(gdata.end(), data.begin(), data.end());
                    }
                    mac = d_crypto->generateMAC(gdata);
                }
                edata = data;

                if (d_crypto->d_auth_method == CM_LEGACY) // Only in native mode mac needs to be added
                {
                    edata.insert(edata.end(), mac.begin(), mac.end());
                }
            }
                break;

            case CM_ENCRYPT:
            {
                std::vector<unsigned char> encparameters;
                encparameters.push_back(cmd);
                if (parameters != NULL)
                {
                    encparameters.insert(encparameters.end(), parameters, parameters + paramLength);
                }
                edata = d_crypto->desfireEncrypt(data, encparameters);
            }
                break;

            default:
                THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Unknown DESFire crypto mode.");
            }
        }
        else
        {
            switch (mode)
            {
            case CM_PLAIN:
                edata = std::vector<unsigned char>(data.begin(), data.begin() + DESFIRE_EV1_CLEAR_DATA_LENGTH_CHUNK);
                if (d_crypto->d_auth_method != CM_LEGACY) // CMAC needs to be recalculated
                {
                    std::vector<unsigned char> apdu_command;
                    apdu_command.push_back(cmd);
                    if (parameters != NULL)
                    {
                        apdu_command.insert(apdu_command.end(), parameters, parameters + paramLength);
                    }
                    if (data.size() > 0)
                    {
                        apdu_command.insert(apdu_command.end(), data.begin(), data.end());
                    }
                    d_crypto->desfire_cmac(apdu_command);
                }
                break;

            case CM_MAC:
                edata = std::vector<unsigned char>(data.begin(), data.begin() + DESFIRE_EV1_CLEAR_DATA_LENGTH_CHUNK);
                d_crypto->bufferingForGenerateMAC(edata);
                break;

            case CM_ENCRYPT:
            {
                edata = d_crypto->encipherData(false, std::vector<unsigned char>(data.begin(), data.begin() + DESFIRE_EV1_CLEAR_DATA_LENGTH_CHUNK));
            }
                break;

            default:
                THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Unknown DESFire crypto mode.");
            }
        }

        if (parameters != NULL && paramLength > 0)
        {
            command.insert(command.end(), parameters, parameters + paramLength);
        }
        command.insert(command.end(), edata.begin(), edata.end());

        std::vector<unsigned char> result = transmit_plain(cmd, command);
        unsigned char err = result[result.size() - 1];
        if (data.size() > DESFIRE_EV1_CLEAR_DATA_LENGTH_CHUNK)
        {
            size_t pos = DESFIRE_EV1_CLEAR_DATA_LENGTH_CHUNK;
            while (err == DF_INS_ADDITIONAL_FRAME) // && pos < dataLength
            {
                size_t pkSize = ((data.size() - pos) >= (DESFIRE_EV1_CLEAR_DATA_LENGTH_CHUNK)) ? (DESFIRE_EV1_CLEAR_DATA_LENGTH_CHUNK) : (data.size() - pos);

                switch (mode)
                {
                case CM_PLAIN:
                    edata = std::vector<unsigned char>(data.begin() + pos, data.begin() + pos + pkSize);
                    break;

                case CM_MAC:
                    edata = std::vector<unsigned char>(data.begin() + pos, data.begin() + pos + pkSize);
                    if (pos + pkSize == data.size())
                    {
                        std::vector<unsigned char> mac;
                        if (d_crypto->d_auth_method == CM_LEGACY)
                        {
                            mac = d_crypto->generateMAC(edata);
                        }
                        else
                        {
                            std::vector<unsigned char> gdata;
                            gdata.push_back(cmd);
                            if (parameters != NULL)
                            {
                                gdata.insert(gdata.end(), (unsigned char*)parameters, (unsigned char*)parameters + paramLength);
                            }
                            gdata.insert(gdata.end(), edata.begin(), edata.end());
                            mac = d_crypto->generateMAC(gdata);
                        }

                        if (d_crypto->d_auth_method == CM_LEGACY) // Only in native mode mac needs to be added
                        {
                            edata.insert(edata.begin(), mac.begin(), mac.end());
                        }
                    }
                    else
                    {
                        d_crypto->bufferingForGenerateMAC(edata);
                    }
                    break;

                case CM_ENCRYPT:
                    if (pos + pkSize == data.size())
                    {
                        std::vector<unsigned char> encparameters;
                        encparameters.push_back(cmd);
                        if (parameters != NULL)
                        {
                            encparameters.insert(encparameters.end(), (unsigned char*)parameters, (unsigned char*)parameters + paramLength);
                        }
                        edata = d_crypto->encipherData(true, std::vector<unsigned char>(data.begin() + pos, data.begin() + pos + pkSize), encparameters);
                    }
                    else
                    {
                        edata = d_crypto->encipherData(false, std::vector<unsigned char>(data.begin() + pos, data.begin() + pos + pkSize));
                    }
                    break;

                default:
                    THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Unknown DESFire crypto mode.");
                }

                result = transmit_plain(DF_INS_ADDITIONAL_FRAME, edata);
                err = result.back();

                pos += pkSize;
            }
        }

        if (err != 0x00)
        {
            char msgtmp[64];
            sprintf(msgtmp, "Unknown error: %x", err);
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, msgtmp);
        }

        if (err == 0x00 && result.size() > 2 && (mode == CM_MAC || mode == CM_ENCRYPT || d_crypto->d_auth_method != CM_LEGACY))
        {
            result.resize(result.size() - 2);
            if (d_crypto->verifyMAC(true, result))
            {
                //THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "MAC verification failed.");
            }
        }
    }

    unsigned int DESFireEV1ISO7816Commands::readData(unsigned char fileno, unsigned int offset, unsigned int length, void* data, EncryptionMode mode)
    {
        unsigned int ret = 0;

        std::vector<unsigned char> command(7);

        command[0] = static_cast<unsigned char>(fileno);

        // Currently we have some problems to read more than 253 bytes with an Omnikey Reader.
        // So the read command is separated to some commands, 8 bytes aligned.

        for (size_t i = 0; i < length; i += 248)
        {
            size_t trunloffset = offset + i;
            size_t trunklength = ((length - i) > 248) ? 248 : (length - i);
            memcpy(&command[1], &trunloffset, 3);
            memcpy(&command[4], &trunklength, 3);

            std::vector<unsigned char> result = transmit_nomacv(DF_INS_READ_DATA, command);
            unsigned char err = result.back();
            result.resize(result.size() - 2);
            result = handleReadData(err, result, static_cast<unsigned int>(trunklength), mode);
            memcpy(reinterpret_cast<unsigned char*>(data)+i, &result[0], result.size());
            ret += static_cast<unsigned int>(result.size());
        }

        return ret;
    }

    unsigned int DESFireEV1ISO7816Commands::readRecords(unsigned char fileno, unsigned int offset, unsigned int length, void* data, EncryptionMode mode)
    {
        std::vector<unsigned char> command;

        command.push_back(fileno);
        command.insert(command.end(), offset, offset + 3);
        command.insert(command.end(), length, length + 3);

        std::vector<unsigned char> result = transmit_nomacv(DF_INS_READ_RECORDS, command);
        unsigned char err = result.back();
        result.resize(result.size() - 2);
        result = handleReadData(err, result, length, mode);
        memcpy(data, &result[0], result.size());

        return static_cast<unsigned int>(result.size());
    }

    void DESFireEV1ISO7816Commands::changeFileSettings(unsigned char fileno, EncryptionMode comSettings, DESFireAccessRights accessRights, bool plain)
    {
        std::vector<unsigned char> command;
        short ar = AccessRightsInMemory(accessRights);

        command.push_back(static_cast<unsigned char>(comSettings));
        BufferHelper::setUShort(command, ar);

        if (!plain)
        {
            std::vector<unsigned char> param;
            param.push_back(DF_INS_CHANGE_FILE_SETTINGS);
            param.push_back(static_cast<unsigned char>(fileno));
            command = d_crypto->desfireEncrypt(command, param);
        }
        unsigned char uf = static_cast<unsigned char>(fileno);
        command.insert(command.begin(), &uf, &uf + 1);

        if (d_crypto->d_auth_method == CM_LEGACY && plain)
        {
            DESFireISO7816Commands::transmit(DF_INS_CHANGE_FILE_SETTINGS, command);
        }
        else
        {
            std::vector<unsigned char> dd = transmit_plain(DF_INS_CHANGE_FILE_SETTINGS, command);
            dd.resize(dd.size() - 2);
            if (!d_crypto->verifyMAC(true, dd))
            {
                THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "MAC verification failed.");
            }
        }
    }

    void DESFireEV1ISO7816Commands::changeKeySettings(DESFireKeySettings settings)
    {
        unsigned char command[1];
        command[0] = static_cast<unsigned char>(settings);

        unsigned char param[1];
        param[0] = DF_INS_CHANGE_KEY_SETTINGS;

        std::vector<unsigned char> cryptogram = d_crypto->desfireEncrypt(std::vector<unsigned char>(command, command + sizeof(command)), std::vector<unsigned char>(param, param + sizeof(param)));

        if (d_crypto->d_auth_method == CM_LEGACY)
        {
            DESFireISO7816Commands::transmit(DF_INS_CHANGE_KEY_SETTINGS, cryptogram);
        }
        else
        {
            std::vector<unsigned char> r = transmit_plain(DF_INS_CHANGE_KEY_SETTINGS, cryptogram);
            r.resize(r.size() - 2);
            if (!d_crypto->verifyMAC(true, r))
            {
                THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "MAC verification failed.");
            }
        }
    }

    void DESFireEV1ISO7816Commands::changeKey(unsigned char keyno, boost::shared_ptr<DESFireKey> key)
    {
        std::vector<unsigned char> diversify;
        if (key->getKeyDiversification())
        {
            key->getKeyDiversification()->initDiversification(d_crypto->getIdentifier(), d_crypto->d_currentAid, key, keyno, diversify);
        }
        std::vector<unsigned char> cryptogram;

        unsigned char keynobyte = keyno;
        if (keyno == 0 && d_crypto->d_currentAid == 0)
        {
            keynobyte |= key->getKeyType();
        }

        if (boost::dynamic_pointer_cast<SAMKeyStorage>(key->getKeyStorage()))
        {
            cryptogram = getChangeKeySAMCryptogram(keyno, key);
        }
        else
        {
            cryptogram = d_crypto->changeKey_PICC(keynobyte, key, diversify);
        }

        std::vector<unsigned char> data;
        data.push_back(keynobyte);
        data.insert(data.end(), cryptogram.begin(), cryptogram.end());
        if (d_crypto->d_auth_method == CM_LEGACY)
        {
            DESFireISO7816Commands::transmit(DF_INS_CHANGE_KEY, data);
        }
        else
        {
            std::vector<unsigned char> dd = transmit_plain(DF_INS_CHANGE_KEY, data);
            unsigned char err = dd.back();
            dd.resize(dd.size() - 2);

            // Don't check CMAC if master key.
            if (dd.size() > 0 && keyno != 0)
            {
                std::vector<unsigned char> CMAC = d_crypto->desfire_cmac(std::vector<unsigned char>(&err, &err + 1));
                EXCEPTION_ASSERT_WITH_LOG(dd == CMAC, LibLogicalAccessException, "Wrong CMAC.");
            }
        }
    }

    void DESFireEV1ISO7816Commands::getVersion(DESFireCommands::DESFireCardVersion& dataVersion)
    {
        std::vector<unsigned char> result;
        std::vector<unsigned char> allresult;

        result = transmit_nomacv(DF_INS_GET_VERSION);

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
                        memcpy(reinterpret_cast<char*>(&dataVersion) + 7, &result[0], 7);
                        allresult.insert(allresult.end(), result.begin(), result.end() - 2);

                        result = transmit_plain(DF_INS_ADDITIONAL_FRAME);
                        if (result.size() - 2 >= 14)
                        {
                            if (result[result.size() - 1] == 0x00)
                            {
                                memcpy(reinterpret_cast<char*>(&dataVersion) + 14, &result[0], 14);
                                allresult.insert(allresult.end(), result.begin(), result.end() - 2);

                                if (d_crypto->d_auth_method != CM_LEGACY)
                                {
                                    if (!d_crypto->verifyMAC(true, allresult))
                                    {
                                        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "MAC verification failed.");
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    std::vector<unsigned int> DESFireEV1ISO7816Commands::getApplicationIDs()
    {
        std::vector<unsigned int> aids;
        std::vector<unsigned char> result;

        result = transmit(DF_INS_GET_APPLICATION_IDS);

        while (result[result.size() - 1] == DF_INS_ADDITIONAL_FRAME)
        {
            for (size_t i = 0; i < result.size() - 2; i += 3)
            {
                std::vector<unsigned char> aid(result.begin() + i, result.begin() + i + 3);
                aids.push_back(DESFireLocation::convertAidToUInt(aid));
            }

            result = transmit(DF_INS_ADDITIONAL_FRAME);
        }

        result.resize(result.size() - 2);
        if (d_crypto->d_auth_method != CM_LEGACY)
        {
            result.resize(result.size() - 8); // Remove CMAC
        }

        for (size_t i = 0; i < result.size(); i += 3)
        {
            std::vector<unsigned char> aid(result.begin() + i, result.begin() + i + 3);
            aids.push_back(DESFireLocation::convertAidToUInt(aid));
        }

        return aids;
    }

    std::vector<unsigned char> DESFireEV1ISO7816Commands::getFileIDs()
    {
        std::vector<unsigned char> result = transmit(DF_INS_GET_FILE_IDS);
        result.resize(result.size() - 2);
        if (d_crypto->d_auth_method != CM_LEGACY)
        {
            result.resize(result.size() - 8); // Remove CMAC
        }

        std::vector<unsigned char> files;
        for (size_t i = 0; i < result.size(); ++i)
        {
            files.push_back(result[i]);
        }

        return files;
    }

    void DESFireEV1ISO7816Commands::getValue(unsigned char fileno, EncryptionMode mode, unsigned int& value)
    {
        std::vector<unsigned char> command;
        command.push_back(fileno);

        std::vector<unsigned char> result = transmit_nomacv(DF_INS_GET_VALUE, command);
        unsigned char err = result.back();
        result.resize(result.size() - 2);
        result = handleReadData(err, result, 4, mode);

        if (result.size() >= 4)
        {
            size_t offset = 0;
            value = BufferHelper::getInt32(result, offset);
        }
    }

    void DESFireEV1ISO7816Commands::iso_selectApplication(std::vector<unsigned char> isoaid)
    {
        DESFireISO7816Commands::getISO7816ReaderCardAdapter()->sendAPDUCommand(DFEV1_CLA_ISO_COMPATIBLE, ISO7816_INS_SELECT_FILE, SELECT_FILE_BY_AID, 0x00, static_cast<unsigned char>(isoaid.size()), isoaid);
    }

    void DESFireEV1ISO7816Commands::setConfiguration(bool formatCardEnabled, bool randomIdEnabled)
    {
        unsigned char command[1];
        //command[0] = 0x00;
        command[0] = 0x00 | ((formatCardEnabled) ? 0x00 : 0x01) | ((randomIdEnabled) ? 0x02 : 0x00);

        d_crypto->initBuf(sizeof(command));
        std::vector<unsigned char> encBuffer = d_crypto->desfire_encrypt(d_crypto->d_sessionKey, std::vector<unsigned char>(command, command + sizeof(command)));
        std::vector<unsigned char> buf;
        buf.push_back(0x00);
        buf.insert(buf.end(), encBuffer.begin(), encBuffer.end());
        DESFireISO7816Commands::transmit(DFEV1_INS_SET_CONFIGURATION, buf);
    }

    void DESFireEV1ISO7816Commands::setConfiguration(boost::shared_ptr<DESFireKey> defaultKey)
    {
        if (defaultKey->getLength() < 24)
        {
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "The default key length must be 24-byte long.");
        }

        unsigned char command[25];
        memcpy(&command[0], defaultKey->getData(), 24);
        command[24] = defaultKey->getKeyVersion();

        std::vector<unsigned char> encBuffer = d_crypto->desfireEncrypt(std::vector<unsigned char>(command, command + sizeof(command)));
        std::vector<unsigned char> buf;
        buf.push_back(0x02);
        buf.insert(buf.end(), encBuffer.begin(), encBuffer.end());
        DESFireISO7816Commands::transmit(DFEV1_INS_SET_CONFIGURATION, buf);
    }

    void DESFireEV1ISO7816Commands::setConfiguration(const std::vector<unsigned char>& ats)
    {
        std::vector<unsigned char> encBuffer = d_crypto->desfireEncrypt(ats);
        std::vector<unsigned char> buf;
        buf.push_back(0x02);
        buf.insert(buf.end(), encBuffer.begin(), encBuffer.end());
        DESFireISO7816Commands::transmit(DFEV1_INS_SET_CONFIGURATION, buf);
    }

    std::vector<unsigned char> DESFireEV1ISO7816Commands::transmit(unsigned char cmd, const std::vector<unsigned char>& buf, unsigned char lc, bool forceLc)
    {
        std::vector<unsigned char> CMAC;

        if (d_crypto->d_auth_method != CM_LEGACY && cmd != DF_INS_ADDITIONAL_FRAME)
        {
            std::vector<unsigned char> apdu_command;
            apdu_command.push_back(cmd);
            apdu_command.insert(apdu_command.end(), buf.begin(), buf.end());
            CMAC = d_crypto->desfire_cmac(apdu_command);
        }

        std::vector<unsigned char> r = DESFireISO7816Commands::transmit(cmd, buf, lc, forceLc);
        unsigned char err = r.back();
        if (d_crypto->d_auth_method != CM_LEGACY && cmd != DF_INS_ADDITIONAL_FRAME && r.size() > 8 && err != DF_INS_ADDITIONAL_FRAME)
        {
            std::vector<unsigned char> response = std::vector<unsigned char>(r.begin(), r.end() - 10);
            response.push_back(err);

            CMAC = d_crypto->desfire_cmac(response);

            EXCEPTION_ASSERT_WITH_LOG(std::vector<unsigned char>(r.end() - 10, r.end() - 2) == CMAC, LibLogicalAccessException, "Wrong CMAC.");
        }

        return r;
    }

    std::vector<unsigned char> DESFireEV1ISO7816Commands::transmit_plain(unsigned char cmd, unsigned char lc)
    {
        return transmit_plain(cmd, std::vector<unsigned char>(), lc, true);
    }

    std::vector<unsigned char> DESFireEV1ISO7816Commands::transmit_plain(unsigned char cmd, const std::vector<unsigned char>& buf, unsigned char lc, bool forceLc)
    {
        return DESFireISO7816Commands::transmit(cmd, buf, lc, forceLc);
    }

    std::vector<unsigned char> DESFireEV1ISO7816Commands::transmit_nomacv(unsigned char cmd, unsigned char lc)
    {
        return transmit_nomacv(cmd, std::vector<unsigned char>(), lc, true);
    }

    std::vector<unsigned char> DESFireEV1ISO7816Commands::transmit_nomacv(unsigned char cmd, const std::vector<unsigned char>& buf, unsigned char lc, bool forceLc)
    {
        if (d_crypto->d_auth_method != CM_LEGACY && cmd != DF_INS_ADDITIONAL_FRAME)
        {
            std::vector<unsigned char> apdu_command;
            apdu_command.push_back(cmd);
            apdu_command.insert(apdu_command.end(), buf.begin(), buf.end());
            d_crypto->desfire_cmac(apdu_command);
        }

        return DESFireISO7816Commands::transmit(cmd, buf, lc, forceLc);
    }
}