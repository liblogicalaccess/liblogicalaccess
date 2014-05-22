/**
 * \file desfireiso7816commands.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief DESFire ISO7816 commands.
 */

#include "../commands/desfireiso7816commands.hpp"
#include "desfirechip.hpp"
#include "samav1iso7816commands.hpp"
#include "logicalaccess/cards/samkeystorage.hpp"
#include "nxpav2keydiversification.hpp"

#include <cstring>

namespace logicalaccess
{
    DESFireISO7816Commands::DESFireISO7816Commands()
        : DESFireCommands()
    {
        d_crypto.reset(new DESFireCrypto());
    }

    DESFireISO7816Commands::~DESFireISO7816Commands()
    {
    }

    void DESFireISO7816Commands::erase()
    {		
        transmit(0xFC);
    }

    void DESFireISO7816Commands::getVersion(DESFireCardVersion& dataVersion)
    {
        std::vector<unsigned char> result;

        result = transmit(DF_INS_GET_VERSION);

        if ((result.size() - 2) == 7)
        {
            if (result[result.size() - 1] == DF_INS_ADDITIONAL_FRAME)
            {
                memcpy(&dataVersion, &result[0], 7);

                result = transmit(DF_INS_ADDITIONAL_FRAME);

                if (result.size()-2 == 7)
                {
                    if (result[result.size() - 1] == DF_INS_ADDITIONAL_FRAME)
                    {
                        memcpy(reinterpret_cast<char*>(&dataVersion) + 7, &result[0], 7);						

                        result = transmit(DF_INS_ADDITIONAL_FRAME);
                        if (result.size()-2 == 14)
                        {
                            if (result[result.size() - 1] == 0x00)
                            {
                                memcpy(reinterpret_cast<char*>(&dataVersion) + 14, &result[0], 14);
                            }
                        }
                    }
                }
            }
        }
    }

    void DESFireISO7816Commands::selectApplication(unsigned int aid)
    {
        std::vector<unsigned char> command;//, samaid;
        DESFireLocation::convertUIntToAid(aid, command);

        transmit(DF_INS_SELECT_APPLICATION, command);

       /* if (getSAMChip())
        {
            LOG(LogLevel::INFOS) << ) << , "SelectApplication on SAM chip...");
			DESFireLocation::convertUIntToAid(aid, samaid);
			std::reverse(samaid.begin(), samaid.end());
			if (getSAMChip()->getCardType() == "SAM_AV1")
				boost::dynamic_pointer_cast<SAMCommands<KeyEntryAV1Information, SETAV1> >(getSAMChip()->getCommands())->selectApplication(samaid);
			else if (getSAMChip()->getCardType() == "SAM_AV2")
				boost::dynamic_pointer_cast<SAMCommands<KeyEntryAV2Information, SETAV2> >(getSAMChip()->getCommands())->selectApplication(samaid);
        }*/

        d_crypto->selectApplication(aid);
    }

    void DESFireISO7816Commands::createApplication(unsigned int aid, DESFireKeySettings settings, unsigned char maxNbKeys)
    {
        std::vector<unsigned char> command;

        DESFireLocation::convertUIntToAid(aid, command);
		command.push_back(static_cast<unsigned char>(settings));
		command.push_back(static_cast<unsigned char>(maxNbKeys));

        transmit(DF_INS_CREATE_APPLICATION, command, sizeof(command));
    }

    void DESFireISO7816Commands::deleteApplication(unsigned int aid)
    {
       std::vector<unsigned char> command;

        DESFireLocation::convertUIntToAid(aid, command);

        transmit(DF_INS_DELETE_APPLICATION, command, sizeof(command));
    }

    std::vector<unsigned int> DESFireISO7816Commands::getApplicationIDs()
    {
        std::vector<unsigned int> aids;
        std::vector<unsigned char> result;

        result = transmit(DF_INS_GET_APPLICATION_IDS);

        while (result[result.size() - 1] == DF_INS_ADDITIONAL_FRAME)
        {
            for (size_t i = 0; i < result.size() - 2; i += 3)
            {
				std::vector<unsigned char> aid(result[i], result[i + 3]);
                aids.push_back(DESFireLocation::convertAidToUInt(aid));
            }

            result = transmit(DF_INS_ADDITIONAL_FRAME);
        }

        for (size_t i = 0; i < result.size()-2; i += 3)
        {
			std::vector<unsigned char> aid(result[i], result[i + 3]);
            aids.push_back(DESFireLocation::convertAidToUInt(aid));
        }

        return aids;
    }

    std::vector<unsigned char> DESFireISO7816Commands::getChangeKeySAMCryptogram(unsigned char keyno, boost::shared_ptr<DESFireKey> key)
    {
        boost::shared_ptr<SAMKeyStorage> samsks = boost::dynamic_pointer_cast<SAMKeyStorage>(key->getKeyStorage());
		boost::shared_ptr<SAMCommands<KeyEntryAV1Information, SETAV1> > samav1commands = boost::dynamic_pointer_cast<SAMCommands<KeyEntryAV1Information, SETAV1> >(getSAMChip()->getCommands());
		boost::shared_ptr<SAMCommands<KeyEntryAV2Information, SETAV2> > samav2commands = boost::dynamic_pointer_cast<SAMCommands<KeyEntryAV2Information, SETAV2> >(getSAMChip()->getCommands());

        boost::shared_ptr<DESFireProfile> dprofile = boost::dynamic_pointer_cast<DESFireProfile>(getChip()->getProfile());
        boost::shared_ptr<DESFireKey> oldkey = boost::dynamic_pointer_cast<DESFireKey>(dprofile->getKey(d_crypto->d_currentAid, keyno));

        ChangeKeyInfo samck;
        memset(&samck, 0x00, sizeof(samck));
        samck.currentKeySlotNo = 0;
        samck.isMasterKey = (d_crypto->d_currentAid == 0 && keyno == 0x00) ? 1 : 0;
        samck.newKeySlotNo = samsks->getKeySlot();
        samck.newKeySlotV = key->getKeyVersion();
        samck.desfireNumber = keyno;
        if ((d_crypto->d_currentKeyNo == 0 && keyno == 0) || (d_crypto->d_currentKeyNo == keyno /* && ChangeKey key == 0xE */))
        {
            samck.oldKeyInvolvement = 0;
        }
        else
        {
            if (oldkey && boost::dynamic_pointer_cast<SAMKeyStorage>(oldkey->getKeyStorage()))
            {
                boost::shared_ptr<SAMKeyStorage> oldsamks = boost::dynamic_pointer_cast<SAMKeyStorage>(oldkey->getKeyStorage());
                samck.currentKeySlotNo = oldsamks->getKeySlot();
                samck.currentKeySlotV = oldkey->getKeyVersion();
                samck.oldKeyInvolvement = 1;
            }
            else
            {
                THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Current key required on SAM to change the key.");
            }
        }

		std::vector<unsigned char> diversify;
        if (key->getKeyDiversification())
            key->getKeyDiversification()->initDiversification(d_crypto->getIdentifier(), d_crypto->d_currentAid, key, keyno, diversify);
		if (oldkey->getKeyDiversification())
            oldkey->getKeyDiversification()->initDiversification(d_crypto->getIdentifier(), d_crypto->d_currentAid, oldkey, keyno, diversify);

		ChangeKeyDiversification keyDiv;
		memset(&keyDiv, 0x00, sizeof(keyDiv));
		if (samav2commands
			&& (boost::dynamic_pointer_cast<NXPAV2KeyDiversification>(key->getKeyDiversification())
			|| boost::dynamic_pointer_cast<NXPAV2KeyDiversification>(oldkey->getKeyDiversification())))
		{
			std::vector<unsigned char> diversifyNew, diversifyOld;
			boost::shared_ptr<NXPAV2KeyDiversification> nxpdiv = boost::dynamic_pointer_cast<NXPAV2KeyDiversification>(key->getKeyDiversification());
			boost::shared_ptr<NXPAV2KeyDiversification> oldnxpdiv = boost::dynamic_pointer_cast<NXPAV2KeyDiversification>(oldkey->getKeyDiversification());

			if (nxpdiv)
			{
				nxpdiv->initDiversification(d_crypto->getIdentifier(), d_crypto->d_currentAid, key, keyno, diversifyNew);
				keyDiv.diversifyNew = 0x01;
				keyDiv.divInput = &diversifyNew[0];
				keyDiv.divInputSize = static_cast<unsigned char>(diversifyNew.size());
			}

			if (oldnxpdiv)
			{
				oldnxpdiv->initDiversification(d_crypto->getIdentifier(), d_crypto->d_currentAid, key, keyno, diversifyOld);

				if (nxpdiv && oldnxpdiv && !std::equal(diversifyNew.begin(), diversifyNew.end(), diversifyOld.begin()))
					THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Current and New Key should have the same system identifier.");

				keyDiv.diversifyCurrent = 0x01;
				keyDiv.divInput = &diversifyOld[0];
				keyDiv.divInputSize = static_cast<unsigned char>(diversifyOld.size());
			}

			keyDiv.enableAV2 = 0x01;
		}

        std::vector<unsigned char> ret;
		if (samav1commands)
			ret = samav1commands->changeKeyPICC(samck, keyDiv);
		else
			ret = samav2commands->changeKeyPICC(samck, keyDiv);
        d_crypto->d_lastIV.clear();
        d_crypto->d_lastIV.resize(d_crypto->d_block_size);
        std::copy(ret.end() - d_crypto->d_block_size, ret.end(), d_crypto->d_lastIV.begin());
        return ret;
    }

    void DESFireISO7816Commands::changeKey(unsigned char keyno, boost::shared_ptr<DESFireKey> key)
    {
        std::vector<unsigned char> cryptogram;

        if (boost::dynamic_pointer_cast<SAMKeyStorage>(key->getKeyStorage()))
        {
            cryptogram = getChangeKeySAMCryptogram(keyno, key);
        }
        else
        {
            std::vector<unsigned char> diversify;
            if (key->getKeyDiversification())
            {	
                key->getKeyDiversification()->initDiversification(d_crypto->getIdentifier(), d_crypto->d_currentAid, key, keyno, diversify);
            }
            cryptogram = d_crypto->changeKey_PICC(keyno, key, diversify);
        }

        std::vector<unsigned char> command;
		command.push_back(keyno);
        if (cryptogram.size() > 0)
        {
			command.insert(command.end(), cryptogram.begin(), cryptogram.end());
            transmit(DF_INS_CHANGE_KEY, command);
        }
    }

    std::vector<unsigned char> DESFireISO7816Commands::getFileIDs()
    {
        std::vector<unsigned char> result = transmit(DF_INS_GET_FILE_IDS);
        std::vector<unsigned char> files;

        for (size_t i = 0; i < result.size() - 2; ++i)
        {
            files.push_back(result[i]);
        }

        return files;
    }

    void DESFireISO7816Commands::getKeySettings(DESFireKeySettings& settings, unsigned char& maxNbKeys)
    {
        std::vector<unsigned char> result;

        result = transmit(DF_INS_GET_KEY_SETTINGS);

        EXCEPTION_ASSERT_WITH_LOG(result.size() >= 2, LibLogicalAccessException, "incorrect result for transmit");

        settings = (DESFireKeySettings)result[0];
        maxNbKeys = result[1];
    }

    void DESFireISO7816Commands::changeKeySettings(DESFireKeySettings settings)
    {
        unsigned char command[1];
        command[0] = static_cast<unsigned char>(settings);
        std::vector<unsigned char> cryptogram = d_crypto->desfireEncrypt(std::vector<unsigned char>(command, command + sizeof(command)));
        transmit(DF_INS_CHANGE_KEY_SETTINGS, cryptogram);
    }

    void DESFireISO7816Commands::getFileSettings(unsigned char fileno, FileSetting& fileSetting)
    {
        std::vector<unsigned char> command;
		command.push_back(fileno);

        std::vector<unsigned char> result = transmit(DF_INS_GET_FILE_SETTINGS, command);
        memcpy(&fileSetting, &result[0], result.size() - 2);
    }

    std::vector<unsigned char> DESFireISO7816Commands::handleReadData(unsigned char err, const std::vector<unsigned char>& firstMsg, unsigned int length, EncryptionMode mode)
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
            if (mode == CM_MAC)
            {
                if (!d_crypto->verifyMAC(false, data))
                {
                    THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "MAC data doesn't match.");
                }
            }

            data = transmit(DF_INS_ADDITIONAL_FRAME);
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
                
            }
            break;

        case CM_MAC:
            {
                if (!d_crypto->verifyMAC(true, data))
                {
                    THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "MAC data doesn't match.");
                }
                ret.resize(ret.size() - 4);
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

    void DESFireISO7816Commands::handleWriteData(unsigned char cmd, unsigned char* parameters, unsigned int paramLength, const std::vector<unsigned char>& data, EncryptionMode mode)
    {
        std::vector<unsigned char> edata, command(64);

        d_crypto->initBuf(data.size());

        if (data.size() <= DESFIRE_CLEAR_DATA_LENGTH_CHUNK)
        {
            switch (mode)
            {
            case CM_PLAIN:
                {
                    edata = data;
                }
                break;

            case CM_MAC:
                {
                    std::vector<unsigned char> mac = d_crypto->generateMAC(data);
                    edata = data;
                    edata.insert(edata.end(), mac.begin(), mac.end());
                }
                break;

            case CM_ENCRYPT:
                {
                    edata = d_crypto->desfireEncrypt(data);
                }
                break;

            default:
                THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Unknown crypto mode.");
            }			
        }
        else
        {
            switch (mode)
            {
            case CM_PLAIN:
                edata = std::vector<unsigned char>(data.begin(), data.begin() + DESFIRE_CLEAR_DATA_LENGTH_CHUNK);
                break;

            case CM_MAC:
                edata =std::vector<unsigned char>(data.begin(), data.begin() + DESFIRE_CLEAR_DATA_LENGTH_CHUNK);
                d_crypto->bufferingForGenerateMAC(edata);
                break;

            case CM_ENCRYPT:
                edata = d_crypto->encipherData(false, std::vector<unsigned char>(data.begin(), data.begin() + DESFIRE_CLEAR_DATA_LENGTH_CHUNK));
                break;

            default:
                THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Unknown crypto mode.");
            }			
        }

        if (paramLength > 0)
        {
			memcpy(&command[0], parameters, paramLength);
			command.insert(command.end(), parameters, parameters + paramLength);
        }
		size_t p = paramLength;
        memcpy(&command[p], &edata[0], edata.size());
        p += edata.size();
        command[p++] = 0x00;

        std::vector<unsigned char> result = transmit(cmd, command);
        unsigned char err = result.back();
        if (data.size() > DESFIRE_CLEAR_DATA_LENGTH_CHUNK)
        {
            size_t pos = DESFIRE_CLEAR_DATA_LENGTH_CHUNK;
            while (err == DF_INS_ADDITIONAL_FRAME) // && pos < dataLength
            {
                size_t pkSize = ((data.size() - pos) >= DESFIRE_CLEAR_DATA_LENGTH_CHUNK) ? DESFIRE_CLEAR_DATA_LENGTH_CHUNK : (data.size() - pos);

                switch (mode)
                {
                case CM_PLAIN:
                    edata =  std::vector<unsigned char>(data.begin() + pos, data.begin() + pos + pkSize);
                    break;

                case CM_MAC:
                    edata =  std::vector<unsigned char>(data.begin() + pos, data.begin() + pos + pkSize);
                    if (pos + pkSize == data.size())
                    {
                        std::vector<unsigned char> mac = d_crypto->generateMAC(edata);
                        edata.insert(edata.end(), mac.begin(), mac.end());								
                    }
                    else
                    {
                        d_crypto->bufferingForGenerateMAC(edata);
                    }
                    break;

                case CM_ENCRYPT:
                    if (pos + pkSize == data.size())
                    {
                        edata = d_crypto->encipherData(true, std::vector<unsigned char>(data.begin() + pos, data.begin() + pos + pkSize));
                    }
                    else
                    {
                        edata = d_crypto->encipherData(false, std::vector<unsigned char>(data.begin() + pos, data.begin() + pos + pkSize));
                    }
                    break;

                default:
                    THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Unknown crypto mode.");
                }

                result = transmit(DF_INS_ADDITIONAL_FRAME, edata);
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
    }

    void DESFireISO7816Commands::changeFileSettings(unsigned char fileno, EncryptionMode comSettings, DESFireAccessRights accessRights, bool plain)
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
        unsigned char fc = static_cast<unsigned char>(fileno);
        command.insert(command.begin(), &fc, &fc + 1);

        transmit(DF_INS_CHANGE_FILE_SETTINGS, command);
    }

    void DESFireISO7816Commands::createStdDataFile(unsigned char fileno, EncryptionMode comSettings, DESFireAccessRights accessRights, unsigned int fileSize)
    {
        std::vector<unsigned char> command;
        short ar = AccessRightsInMemory(accessRights);

        command.push_back(fileno);
        command.push_back(static_cast<unsigned char>(comSettings));
        BufferHelper::setUShort(command, ar);
        command.push_back(static_cast<unsigned char>(fileSize & 0xff));
        command.push_back(static_cast<unsigned char>(static_cast<unsigned short>(fileSize & 0xff00) >> 8));
        command.push_back(static_cast<unsigned char>(static_cast<unsigned int>(fileSize & 0xff0000) >> 16));

        transmit(DF_INS_CREATE_STD_DATA_FILE, command);
    }

    void DESFireISO7816Commands::createBackupFile(unsigned char fileno, EncryptionMode comSettings, DESFireAccessRights accessRights, unsigned int fileSize)
    {
        std::vector<unsigned char> command;
        short ar = AccessRightsInMemory(accessRights);

        command.push_back(fileno);
        command.push_back(static_cast<unsigned char>(comSettings));
        BufferHelper::setUShort(command, ar);
        command.push_back(static_cast<unsigned char>(fileSize & 0xff));
        command.push_back(static_cast<unsigned char>(static_cast<unsigned short>(fileSize & 0xff00) >> 8));
        command.push_back(static_cast<unsigned char>(static_cast<unsigned int>(fileSize & 0xff0000) >> 16));

        transmit(DF_INS_CREATE_BACKUP_DATA_FILE, command);
    }

    void DESFireISO7816Commands::createValueFile(unsigned char fileno, EncryptionMode comSettings, DESFireAccessRights accessRights, unsigned int lowerLimit, unsigned int upperLimit, unsigned int value, bool limitedCreditEnabled)
    {
        std::vector<unsigned char> command;
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

    void DESFireISO7816Commands::createLinearRecordFile(unsigned char fileno, EncryptionMode comSettings, DESFireAccessRights accessRights, unsigned int fileSize, unsigned int maxNumberOfRecords)
    {
        std::vector<unsigned char> command;
        short ar = AccessRightsInMemory(accessRights);

        command.push_back(fileno);
        command.push_back(static_cast<unsigned char>(comSettings));
        BufferHelper::setUShort(command, ar);
        command.push_back(static_cast<unsigned char>(fileSize & 0xff));
        command.push_back(static_cast<unsigned char>(static_cast<unsigned short>(fileSize & 0xff00) >> 8));
        command.push_back(static_cast<unsigned char>(static_cast<unsigned int>(fileSize & 0xff0000) >> 16));
        command.push_back(static_cast<unsigned char>(maxNumberOfRecords & 0xff));
        command.push_back(static_cast<unsigned char>(static_cast<unsigned short>(maxNumberOfRecords & 0xff00) >> 8));
        command.push_back(static_cast<unsigned char>(static_cast<unsigned int>(maxNumberOfRecords & 0xff0000) >> 16));

        transmit(DF_INS_CREATE_LINEAR_RECORD_FILE, command);
    }

    void DESFireISO7816Commands::createCyclicRecordFile(unsigned char fileno, EncryptionMode comSettings, DESFireAccessRights accessRights, unsigned int fileSize, unsigned int maxNumberOfRecords)
    {
        std::vector<unsigned char> command;
        short ar = AccessRightsInMemory(accessRights);

        command.push_back(static_cast<unsigned char>(fileno));
        command.push_back(static_cast<unsigned char>(comSettings));
        BufferHelper::setUShort(command, ar);
        command.push_back(static_cast<unsigned char>(fileSize & 0xff));
        command.push_back(static_cast<unsigned char>(static_cast<unsigned short>(fileSize & 0xff00) >> 8));
        command.push_back(static_cast<unsigned char>(static_cast<unsigned int>(fileSize & 0xff0000) >> 16));
        command.push_back(static_cast<unsigned char>(maxNumberOfRecords & 0xff));
        command.push_back(static_cast<unsigned char>(static_cast<unsigned short>(maxNumberOfRecords & 0xff00) >> 8));
        command.push_back(static_cast<unsigned char>(static_cast<unsigned int>(maxNumberOfRecords & 0xff0000) >> 16));

        transmit(DF_INS_CREATE_CYCLIC_RECORD_FILE, command);
    }

    void DESFireISO7816Commands::deleteFile(unsigned char fileno)
    {
        std::vector<unsigned char> command;

		command.push_back(fileno);

        transmit(DF_INS_DELETE_FILE, command);
    }

    unsigned int DESFireISO7816Commands::readData(unsigned char fileno, unsigned int offset, unsigned int length, void* data, EncryptionMode mode)
    {
        unsigned int ret = 0;

        std::vector<unsigned char> command;

		command.push_back(fileno);				

        // Currently we have some problems to read more than 253 bytes with an Omnikey Reader.
        // So the read command is separated to some commands, 8 bytes aligned.

        for (size_t i = 0; i < length; i += 248)
        {			
            size_t trunloffset = offset + i;
            size_t trunklength = ((length - i) > 248) ? 248 : (length - i);
			command.push_back(static_cast<unsigned char>(trunloffset & 0xff));
			command.push_back(static_cast<unsigned char>(static_cast<unsigned short>(trunloffset & 0xff00) >> 8));
			command.push_back(static_cast<unsigned char>(static_cast<unsigned int>(trunloffset & 0xff0000) >> 16));
            command.push_back(static_cast<unsigned char>(trunklength & 0xff));
            command.push_back(static_cast<unsigned char>(static_cast<unsigned short>(trunklength & 0xff00) >> 8));
            command.push_back(static_cast<unsigned char>(static_cast<unsigned int>(trunklength & 0xff0000) >> 16));			

            std::vector<unsigned char> result = transmit(DF_INS_READ_DATA, command);
            unsigned char err = result.back();
            result.resize(result.size() - 2);
            result = handleReadData(err, result, static_cast<unsigned int>(trunklength), mode);
            memcpy(reinterpret_cast<unsigned char*>(data) + i, &result[0], result.size());
            ret += static_cast<unsigned int>(result.size());
        }

        return ret;
    }

    void DESFireISO7816Commands::writeData(unsigned char fileno, unsigned int offset, unsigned int length, const void* data, EncryptionMode mode)
    {
        unsigned char parameters[7];
        parameters[0] = static_cast<unsigned char>(fileno);
        parameters[1] = static_cast<unsigned char>(offset & 0xff);
        parameters[2] = static_cast<unsigned char>(static_cast<unsigned short>(offset & 0xff00) >> 8);
        parameters[3] = static_cast<unsigned char>(static_cast<unsigned int>(offset & 0xff0000) >> 16);
        parameters[4] = static_cast<unsigned char>(length & 0xff);
        parameters[5] = static_cast<unsigned char>(static_cast<unsigned short>(length & 0xff00) >> 8);
        parameters[6] = static_cast<unsigned char>(static_cast<unsigned int>(length & 0xff0000) >> 16);

        handleWriteData(DF_INS_WRITE_DATA, parameters, static_cast<unsigned int>(sizeof(parameters)), std::vector<unsigned char>((unsigned char*)data, (unsigned char*)data + length), mode);
    }

    void DESFireISO7816Commands::getValue(unsigned char fileno, EncryptionMode mode, unsigned int& value)
    {
        std::vector<unsigned char> command;
		command.push_back(fileno);

        std::vector<unsigned char> result = transmit(DF_INS_GET_VALUE, command);
        unsigned char err = result.back();
        result.resize(result.size() - 2);
        result = handleReadData(err, result, 4, mode);

        if (result.size() >= 4)
        {
            size_t offset = 0;
            value = BufferHelper::getUInt32(result, offset);
        }
    }

    void DESFireISO7816Commands::credit(unsigned char fileno, unsigned int value, EncryptionMode mode)
    {
        unsigned char parameters[1];
        parameters[0] = fileno;

        handleWriteData(DF_INS_CREDIT, parameters, static_cast<unsigned int>(sizeof(parameters)), std::vector<unsigned char>(&value, &value + 4), mode);
    }

    void DESFireISO7816Commands::debit(unsigned char fileno, unsigned int value, EncryptionMode mode)
    {
        unsigned char parameters[1];
        parameters[0] = fileno;

        handleWriteData(DF_INS_DEBIT, parameters, static_cast<unsigned int>(sizeof(parameters)), std::vector<unsigned char>(&value, &value + 4), mode);
    }

    void DESFireISO7816Commands::limitedCredit(unsigned char fileno, unsigned int value, EncryptionMode mode)
    {
        unsigned char parameters[1];
        parameters[0] = static_cast<unsigned char>(fileno);

        handleWriteData(DF_INS_LIMITED_CREDIT, parameters, static_cast<unsigned int>(sizeof(parameters)), std::vector<unsigned char>(&value, &value + 4), mode);
    }

    void DESFireISO7816Commands::writeRecord(unsigned char fileno, unsigned int offset, unsigned int length, const void* data, EncryptionMode mode)
    {
        unsigned char parameters[7];
        parameters[0] = fileno;
        parameters[1] = static_cast<unsigned char>(offset & 0xff);
        parameters[2] = static_cast<unsigned char>(static_cast<unsigned short>(offset & 0xff00) >> 8);
        parameters[3] = static_cast<unsigned char>(static_cast<unsigned int>(offset & 0xff0000) >> 16);
        parameters[4] = static_cast<unsigned char>(length & 0xff);
        parameters[5] = static_cast<unsigned char>(static_cast<unsigned short>(length & 0xff00) >> 8);
        parameters[6] = static_cast<unsigned char>(static_cast<unsigned int>(length & 0xff0000) >> 16);

        handleWriteData(DF_INS_WRITE_RECORD, parameters, static_cast<unsigned int>(sizeof(parameters)), std::vector<unsigned char>((unsigned char*)data, (unsigned char*)data + length), mode);
    }

    unsigned int DESFireISO7816Commands::readRecords(unsigned char fileno, unsigned int offset, unsigned int length, void* data, EncryptionMode mode)
    {
        std::vector<unsigned char> command;

		command.push_back(fileno);
        command.push_back(static_cast<unsigned char>(offset & 0xff));
        command.push_back(static_cast<unsigned char>(static_cast<unsigned short>(offset & 0xff00) >> 8));
        command.push_back(static_cast<unsigned char>(static_cast<unsigned int>(offset & 0xff0000) >> 16));
        command.push_back(static_cast<unsigned char>(length & 0xff));
        command.push_back(static_cast<unsigned char>(static_cast<unsigned short>(length & 0xff00) >> 8));
        command.push_back(static_cast<unsigned char>(static_cast<unsigned int>(length & 0xff0000) >> 16));

        std::vector<unsigned char> result = transmit(DF_INS_READ_RECORDS, command);
        unsigned char err = result.back();
        result.resize(result.size() - 2);
        result = handleReadData(err, result, length, mode);
        memcpy(data, &result[0], result.size());
        
        return static_cast<unsigned int>(result.size());
    }

    void DESFireISO7816Commands::clearRecordFile(unsigned char fileno)
    {
        std::vector<unsigned char> command;
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
        boost::shared_ptr<DESFireKey> key = d_crypto->getKey(keyno);
        authenticate(keyno, key);
    }

    void DESFireISO7816Commands::authenticate(unsigned char keyno, boost::shared_ptr<DESFireKey> key)
    {
        std::vector<unsigned char> command;

		boost::shared_ptr<DESFireProfile> dprofile = boost::dynamic_pointer_cast<DESFireProfile>(getChip()->getProfile());
		if (!key) {
			key = dprofile->getDefaultKey(DF_KEY_DES);
		}
		dprofile->setKey(d_crypto->d_currentAid, keyno, key);

        if (boost::dynamic_pointer_cast<SAMKeyStorage>(key->getKeyStorage()) && !getSAMChip())
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "SAMKeyStorage set on the key but no SAM reader has been set.");

        std::vector<unsigned char> diversify;
        if (key->getKeyDiversification())
        {
            key->getKeyDiversification()->initDiversification(d_crypto->getIdentifier(), d_crypto->d_currentAid, key, keyno, diversify);
        }
        command.push_back(keyno);

        std::vector<unsigned char> result = DESFireISO7816Commands::transmit(DF_INS_AUTHENTICATE, command);
        if (result[result.size() - 1] == DF_INS_ADDITIONAL_FRAME && (result.size()-2) >= 8)
        {
            result.resize(8);
            std::vector<unsigned char> rndAB, apduresult;

            if (boost::dynamic_pointer_cast<SAMKeyStorage>(key->getKeyStorage()))
            {
                std::vector<unsigned char> data(2 + result.size());
                data[0] = keyno;
                data[1] = key->getKeyVersion();
                memcpy(&data[0] + 2, &result[0], result.size());

				unsigned char cmd[] = { 0x80, 0x0a, 0x02, 0x00, static_cast<unsigned char>(data.size()), 0x00 };
				std::vector<unsigned char> cmd_vector(cmd, cmd + 6);
				cmd_vector.insert(cmd_vector.end() - 1, data.begin(), data.end());
				if (getSAMChip()->getCardType() == "SAM_AV1")
					apduresult = boost::dynamic_pointer_cast<SAMCommands<KeyEntryAV1Information, SETAV1> >(getSAMChip()->getCommands())->transmit(cmd_vector);
				else if (getSAMChip()->getCardType() == "SAM_AV2")
					apduresult = boost::dynamic_pointer_cast<SAMCommands<KeyEntryAV2Information, SETAV2> >(getSAMChip()->getCommands())->transmit(cmd_vector, true, false);
                if (apduresult.size() <= 2)
                    THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "sam authenticate DES P1 failed.");

                rndAB.insert(rndAB.begin(), apduresult.begin(), apduresult.begin() + 16);
            }
            else
                rndAB = d_crypto->authenticate_PICC1(keyno, diversify, result);
            result = DESFireISO7816Commands::transmit(DF_INS_ADDITIONAL_FRAME, rndAB);
            if ((result.size() - 2) >= 8)
            {
                result.resize(result.size() - 2);
                    
                if (boost::dynamic_pointer_cast<SAMKeyStorage>(key->getKeyStorage()))
                {
					unsigned char cmd[] = { 0x80, 0x0a, 0x00, 0x00, 0x08, 0x08 };
					std::vector<unsigned char> cmd_vector(cmd, cmd + 6);
					cmd_vector.insert(cmd_vector.end() - 1, result.begin(), result.end());
					if (getSAMChip()->getCardType() == "SAM_AV1")
						apduresult = boost::dynamic_pointer_cast<SAMCommands<KeyEntryAV1Information, SETAV1> >(getSAMChip()->getCommands())->transmit(cmd_vector);
					else if (getSAMChip()->getCardType() == "SAM_AV2")
						apduresult = boost::dynamic_pointer_cast<SAMCommands<KeyEntryAV2Information, SETAV2> >(getSAMChip()->getCommands())->transmit(cmd_vector, true, false);
					if (apduresult.size() != 2 || apduresult[0] != 0x90 || apduresult[1] != 0x00)
                        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "sam authenticate DES P2 failed.");
					if (getSAMChip()->getCardType() == "SAM_AV1")
						d_crypto->d_sessionKey = boost::dynamic_pointer_cast<SAMCommands<KeyEntryAV1Information, SETAV1> >(getSAMChip()->getCommands())->dumpSessionKey();
					else if (getSAMChip()->getCardType() == "SAM_AV2")
						d_crypto->d_sessionKey = boost::dynamic_pointer_cast<SAMCommands<KeyEntryAV2Information, SETAV2> >(getSAMChip()->getCommands())->dumpSessionKey();
                    d_crypto->d_currentKeyNo = keyno;
                }
                else
                    d_crypto->authenticate_PICC2(keyno, result);
            }
        }
    }

    std::vector<unsigned char> DESFireISO7816Commands::transmit(unsigned char cmd, unsigned char lc)
    {
        return transmit(cmd, std::vector<unsigned char>(), lc, true);
    }

    std::vector<unsigned char> DESFireISO7816Commands::transmit(unsigned char cmd, const std::vector<unsigned char>& data, unsigned char lc, bool forceLc)
    {		
		if (data.size())
        {
            return getISO7816ReaderCardAdapter()->sendAPDUCommand(DF_CLA_ISO_WRAP, cmd, 0x00, 0x00, static_cast<unsigned char>(data.size()), data, 0x00);
        }
        else if (forceLc)
        {
            return getISO7816ReaderCardAdapter()->sendAPDUCommand(DF_CLA_ISO_WRAP, cmd, 0x00, 0x00, lc, 0x00);
        }

        return getISO7816ReaderCardAdapter()->sendAPDUCommand(DF_CLA_ISO_WRAP, cmd, 0x00, 0x00, 0x00);
    }
}
