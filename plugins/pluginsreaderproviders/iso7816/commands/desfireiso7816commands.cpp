/**
 * \file desfireiso7816commands.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief DESFire ISO7816 commands.
 */

#include "../commands/desfireiso7816commands.hpp"
#include "desfirechip.hpp"
#include "samav1iso7816commands.hpp"
#include "logicalaccess/cards/samkeystorage.hpp"

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

	bool DESFireISO7816Commands::erase(bool resetKey)
	{		
		bool r = false;

		if (selectApplication(0))
		{
			if (authenticate(0))
			{
				transmit(0xFC);
				if (resetKey)
				{
					r = changeKey(0, boost::shared_ptr<DESFireKey>(new DESFireKey(string("00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00"))));
				}
			}
			else
			{
				THROW_EXCEPTION_WITH_LOG(CardException, EXCEPTION_MSG_AUTHENTICATE);
			}
		}
		else
		{
			THROW_EXCEPTION_WITH_LOG(CardException, EXCEPTION_MSG_SELECTAPPLICATION);
		}

		return r;
	}

	bool DESFireISO7816Commands::getVersion(DESFireCardVersion& dataVersion)
	{
		bool r = false;
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

								r = true;
							}
						}
					}
				}
			}
		}

		return r;
	}

	bool DESFireISO7816Commands::selectApplication(int aid)
	{
		bool r = false;
		unsigned char command[3];
		DESFireLocation::convertIntToAid(aid, command);

		transmit(DF_INS_SELECT_APPLICATION, command, sizeof(command));

		if (getSAMChip())
		{
			boost::shared_ptr<SAMCommands> samcommands = boost::dynamic_pointer_cast<SAMCommands>(getSAMChip()->getCommands());
			unsigned char t_aid[3] = {};
			int saveaid = aid;
			for (char x = 2; x >= 0; --x)
			{
				t_aid[(unsigned char)x] = saveaid & 0xff;
				saveaid >>= 8;
			}
			samcommands->selectApplication(t_aid);
		}

		d_crypto->selectApplication(aid);
		r = true;

		return r;
	}

	bool DESFireISO7816Commands::createApplication(int aid, DESFireKeySettings settings, int maxNbKeys)
	{
		bool r = false;
		unsigned char command[5];

		DESFireLocation::convertIntToAid(aid, command);
		command[3] = static_cast<unsigned char>(settings);
		command[4] = static_cast<unsigned char>(maxNbKeys);

		transmit(DF_INS_CREATE_APPLICATION, command, sizeof(command));
		r = true;

		return r;
	}

	bool DESFireISO7816Commands::deleteApplication(int aid)
	{
		bool r = false;
		unsigned char command[3];

		DESFireLocation::convertIntToAid(aid, command);

		transmit(DF_INS_DELETE_APPLICATION, command, sizeof(command));
		r = true;

		return r;
	}

	std::vector<int> DESFireISO7816Commands::getApplicationIDs()
	{
		std::vector<int> aids;
		std::vector<unsigned char> result;

		result = transmit(DF_INS_GET_APPLICATION_IDS);

		while (result[result.size() - 1] == DF_INS_ADDITIONAL_FRAME)
		{
			for (size_t i = 0; i < result.size() - 2; i += 3)
			{
				aids.push_back(DESFireLocation::convertAidToInt(&result[i]));
			}

			result = transmit(DF_INS_ADDITIONAL_FRAME);
		}

		for (size_t i = 0; i < result.size()-2; i += 3)
		{
			aids.push_back(DESFireLocation::convertAidToInt(&result[i]));
		}

		return aids;
	}

	std::vector<unsigned char> DESFireISO7816Commands::getChangeKeySAMCryptogram(unsigned char keyno, boost::shared_ptr<DESFireKey> key)
	{
		boost::shared_ptr<SAMKeyStorage> samsks = boost::dynamic_pointer_cast<SAMKeyStorage>(key->getKeyStorage());
		boost::shared_ptr<SAMCommands> samcommands = boost::dynamic_pointer_cast<SAMCommands>(getSAMChip()->getCommands());

		boost::shared_ptr<DESFireProfile> dprofile = boost::dynamic_pointer_cast<DESFireProfile>(getChip()->getProfile());
		boost::shared_ptr<DESFireKey> oldkey = boost::dynamic_pointer_cast<DESFireKey>(dprofile->getKey(d_crypto->d_currentAid, d_crypto->d_currentKeyNo));

		ChangeKeyInfo samck;
		memset(&samck, 0x00, sizeof(samck));
		samck.currentKeyNo = d_crypto->d_currentKeyNo;
		samck.isMasterKey = (d_crypto->d_currentAid == 0x00 && keyno == 0x00) ? 1 : 0;
		samck.newKeyNo = samsks->getKeySlot();
		samck.newKeyV = key->getKeyVersion();
		samck.desfireNumber = keyno;
		if (oldkey && boost::dynamic_pointer_cast<SAMKeyStorage>(key->getKeyStorage()))
		{
			boost::shared_ptr<SAMKeyStorage> oldsamks = boost::dynamic_pointer_cast<SAMKeyStorage>(key->getKeyStorage());
			samck.currentKeyNo = oldsamks->getKeySlot();
			samck.currentKeyV = oldkey->getKeyVersion();
			samck.oldKeyInvolvement = 1;
		}

		return samcommands->changeKeyPICC(samck);
	}

	bool DESFireISO7816Commands::changeKey(unsigned char keyno, boost::shared_ptr<DESFireKey> key)
	{
		bool r = false;
		std::vector<unsigned char> cryptogram;

		if (boost::dynamic_pointer_cast<SAMKeyStorage>(key->getKeyStorage()))
		{
			cryptogram = getChangeKeySAMCryptogram(keyno, key);
		}
		else
		{
			unsigned char diversify[16];
			if (!key->getDiversify() || d_crypto->getDiversify(diversify))
			{				
				cryptogram = d_crypto->changeKey_PICC(keyno, key, diversify);
			}
		}

		unsigned char command[25];
		command[0] = keyno;
		if (cryptogram.size() > 0)
		{
			memcpy(&command[1], &cryptogram[0], cryptogram.size());

			std::vector<unsigned char> result = transmit(DF_INS_CHANGE_KEY, command, sizeof(command));
			r = true;	
		}

		return r;
	}

	std::vector<int> DESFireISO7816Commands::getFileIDs()
	{
		std::vector<unsigned char> result = transmit(DF_INS_GET_FILE_IDS);
		std::vector<int> files;

		for (size_t i = 0; i < result.size() - 2; ++i)
		{
			files.push_back(result[i]);
		}

		return files;
	}

	void DESFireISO7816Commands::getKeySettings(DESFireKeySettings& settings, unsigned int& maxNbKeys)
	{
		std::vector<unsigned char> result;

		result = transmit(DF_INS_GET_KEY_SETTINGS);

		EXCEPTION_ASSERT_WITH_LOG(result.size() >= 2, LibLogicalAccessException, "incorrect result for transmit");

		settings = (DESFireKeySettings)result[0];
		maxNbKeys = result[1];
	}

	bool DESFireISO7816Commands::changeKeySettings(DESFireKeySettings settings)
	{
		unsigned char command[1];
		command[0] = static_cast<unsigned char>(settings);
		std::vector<unsigned char> cryptogram = d_crypto->desfireEncrypt(std::vector<unsigned char>(command, command + sizeof(command)));
		transmit(DF_INS_CHANGE_KEY_SETTINGS, cryptogram);

		return true;
	}

	bool DESFireISO7816Commands::getFileSettings(int fileno, FileSetting& fileSetting)
	{
		bool r = false;

		unsigned char command[1];
		command[0] = static_cast<unsigned char>(fileno);

		std::vector<unsigned char> result = transmit(DF_INS_GET_FILE_SETTINGS, command, sizeof(command));
		memcpy(&fileSetting, &result[0], result.size()-2);
		r = true;

		return r;
	}

	std::vector<unsigned char> DESFireISO7816Commands::handleReadData(unsigned char err, const std::vector<unsigned char>& firstMsg, size_t length, EncryptionMode mode)
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

	bool DESFireISO7816Commands::handleWriteData(unsigned char cmd, void* parameters, size_t paramLength, const std::vector<unsigned char> data, EncryptionMode mode)
	{
		std::vector<unsigned char> edata;
		unsigned char command[64];
		size_t p = 0;
		size_t pos = 0;
		size_t pkSize = 0;
		bool ret = false;

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
				return false;
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
				return false;
			}			
		}

		if (paramLength > 0)
		{
			memcpy(command, parameters, paramLength);
		}
		p += paramLength;
		memcpy(&command[p], &edata[0], edata.size());
		p += edata.size();
		command[p++] = 0x00;

		std::vector<unsigned char> result = transmit(cmd, command, (paramLength + edata.size()));
		unsigned char err = result.back();
		if (data.size() > DESFIRE_CLEAR_DATA_LENGTH_CHUNK)
		{
			ret = true;
			pos += DESFIRE_CLEAR_DATA_LENGTH_CHUNK;
			while (ret && err == DF_INS_ADDITIONAL_FRAME) // && pos < dataLength
			{
				pkSize = ((data.size() - pos) >= DESFIRE_CLEAR_DATA_LENGTH_CHUNK) ? DESFIRE_CLEAR_DATA_LENGTH_CHUNK : (data.size() - pos);

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
					return false;
				}

				result = transmit(DF_INS_ADDITIONAL_FRAME, edata);
				err = result.back();

				pos += pkSize;
			}			
		}

		ret = (err == 0x00);
		if (!ret)
		{
			COM_("Return an error: %x", err);
		}

		return ret;
	}

	bool DESFireISO7816Commands::changeFileSettings(int fileno, EncryptionMode comSettings, DESFireAccessRights accessRights, bool plain)
	{
		bool r = false;

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
		r = true;

		return r;
	}

	bool DESFireISO7816Commands::createStdDataFile(int fileno, EncryptionMode comSettings, DESFireAccessRights accessRights, int fileSize)
	{
		bool r = false;

		std::vector<unsigned char> command;
		short ar = AccessRightsInMemory(accessRights);

		command.push_back(static_cast<unsigned char>(fileno));
		command.push_back(static_cast<unsigned char>(comSettings));
		BufferHelper::setUShort(command, ar);
		command.push_back(static_cast<unsigned char>(fileSize & 0xff));
		command.push_back(static_cast<unsigned char>(static_cast<unsigned short>(fileSize & 0xff00) >> 8));
		command.push_back(static_cast<unsigned char>(static_cast<unsigned int>(fileSize & 0xff0000) >> 16));

		transmit(DF_INS_CREATE_STD_DATA_FILE, command);
		r = true;

		return r;
	}

	bool DESFireISO7816Commands::createBackupFile(int fileno, EncryptionMode comSettings, DESFireAccessRights accessRights, int fileSize)
	{
		bool r = false;

		std::vector<unsigned char> command;
		short ar = AccessRightsInMemory(accessRights);

		command.push_back(static_cast<unsigned char>(fileno));
		command.push_back(static_cast<unsigned char>(comSettings));
		BufferHelper::setUShort(command, ar);
		command.push_back(static_cast<unsigned char>(fileSize & 0xff));
		command.push_back(static_cast<unsigned char>(static_cast<unsigned short>(fileSize & 0xff00) >> 8));
		command.push_back(static_cast<unsigned char>(static_cast<unsigned int>(fileSize & 0xff0000) >> 16));

		transmit(DF_INS_CREATE_BACKUP_DATA_FILE, command);
		r = true;

		return r;
	}

	bool DESFireISO7816Commands::createValueFile(int fileno, EncryptionMode comSettings, DESFireAccessRights accessRights, unsigned int lowerLimit, unsigned int upperLimit, int value, bool limitedCreditEnabled)
	{
		bool r = false;

		std::vector<unsigned char> command;
		short ar = AccessRightsInMemory(accessRights);

		command.push_back(static_cast<unsigned char>(fileno));
		command.push_back(static_cast<unsigned char>(comSettings));
		BufferHelper::setUShort(command, ar);
		BufferHelper::setInt32(command, static_cast<int>(lowerLimit));
		BufferHelper::setInt32(command, static_cast<int>(upperLimit));
		BufferHelper::setInt32(command, value);
		command.push_back(limitedCreditEnabled ? 0x01 : 0x00);

		transmit(DF_INS_CREATE_VALUE_FILE, command);
		r = true;

		return r;
	}

	bool DESFireISO7816Commands::createLinearRecordFile(int fileno, EncryptionMode comSettings, DESFireAccessRights accessRights, int fileSize, int maxNumberOfRecords)
	{
		bool r = false;

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

		transmit(DF_INS_CREATE_LINEAR_RECORD_FILE, command);
		r = true;

		return r;
	}

	bool DESFireISO7816Commands::createCyclicRecordFile(int fileno, EncryptionMode comSettings, DESFireAccessRights accessRights, int fileSize, int maxNumberOfRecords)
	{
		bool r = false;

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
		r = true;

		return r;
	}

	bool DESFireISO7816Commands::deleteFile(int fileno)
	{
		bool r = false;

		unsigned char command[1];

		command[0] = static_cast<unsigned char>(fileno);

		transmit(DF_INS_DELETE_FILE, command, sizeof(command));
		r = true;

		return r;
	}

	size_t DESFireISO7816Commands::readData(int fileno, size_t offset, size_t length, void* data, EncryptionMode mode)
	{
		size_t ret = 0;

		unsigned char command[7];

		command[0] = static_cast<unsigned char>(fileno);				

		// Currently we have some problems to read more than 253 bytes with an Omnikey Reader.
		// So the read command is separated to some commands, 8 bytes aligned.

		for (size_t i = 0; i < length; i += 248)
		{			
			size_t trunloffset = offset + i;
			size_t trunklength = ((length - i) > 248) ? 248 : (length - i);
			command[1] = static_cast<unsigned char>(trunloffset & 0xff);
			command[2] = static_cast<unsigned char>(static_cast<unsigned short>(trunloffset & 0xff00) >> 8);
			command[3] = static_cast<unsigned char>(static_cast<unsigned int>(trunloffset & 0xff0000) >> 16);
			command[4] = static_cast<unsigned char>(trunklength & 0xff);
			command[5] = static_cast<unsigned char>(static_cast<unsigned short>(trunklength & 0xff00) >> 8);
			command[6] = static_cast<unsigned char>(static_cast<unsigned int>(trunklength & 0xff0000) >> 16);			

			std::vector<unsigned char> result = transmit(DF_INS_READ_DATA, command, sizeof(command));
			unsigned char err = result.back();
			result.resize(result.size() - 2);
			result = handleReadData(err, result, trunklength, mode);
			memcpy(reinterpret_cast<unsigned char*>(data) + i, &result[0], result.size());
			ret += result.size();
		}

		return ret;
	}

	bool DESFireISO7816Commands::writeData(int fileno, size_t offset, size_t length, const void* data, EncryptionMode mode)
	{
		bool ret = false;

		unsigned char parameters[7];
		parameters[0] = static_cast<unsigned char>(fileno);
		parameters[1] = static_cast<unsigned char>(offset & 0xff);
		parameters[2] = static_cast<unsigned char>(static_cast<unsigned short>(offset & 0xff00) >> 8);
		parameters[3] = static_cast<unsigned char>(static_cast<unsigned int>(offset & 0xff0000) >> 16);
		parameters[4] = static_cast<unsigned char>(length & 0xff);
		parameters[5] = static_cast<unsigned char>(static_cast<unsigned short>(length & 0xff00) >> 8);
		parameters[6] = static_cast<unsigned char>(static_cast<unsigned int>(length & 0xff0000) >> 16);

		if (handleWriteData(DF_INS_WRITE_DATA, parameters, sizeof(parameters), std::vector<unsigned char>((unsigned char*)data, (unsigned char*)data + length), mode))
		{
			ret = true;
		}
		
		return ret;
	}

	bool DESFireISO7816Commands::getValue(int fileno, EncryptionMode mode, int& value)
	{
		bool ret = false;

		unsigned char command[1];
		command[0] = static_cast<unsigned char>(fileno);

		std::vector<unsigned char> result = transmit(DF_INS_GET_VALUE, command, sizeof(command));
		unsigned char err = result.back();
		result.resize(result.size() - 2);
		result = handleReadData(err, result, 4, mode);

		if (result.size() >= 4)
		{
			size_t offset = 0;
			value = BufferHelper::getInt32(result, offset);
			ret = true;
		}
		
		return ret;
	}

	bool DESFireISO7816Commands::credit(int fileno, int value, EncryptionMode mode)
	{
		bool ret = false;

		unsigned char parameters[1];
		parameters[0] = static_cast<unsigned char>(fileno);

		if (handleWriteData(DF_INS_CREDIT, parameters, sizeof(parameters), std::vector<unsigned char>(&value, &value + 4), mode))
		{
			ret = true;
		}
		
		return ret;
	}

	bool DESFireISO7816Commands::debit(int fileno, int value, EncryptionMode mode)
	{
		bool ret = false;

		unsigned char parameters[1];
		parameters[0] = static_cast<unsigned char>(fileno);

		if (handleWriteData(DF_INS_DEBIT, parameters, sizeof(parameters), std::vector<unsigned char>(&value, &value + 4), mode))
		{
			ret = true;
		}
		
		return ret;
	}

	bool DESFireISO7816Commands::limitedCredit(int fileno, int value, EncryptionMode mode)
	{
		bool ret = false;

		unsigned char parameters[1];
		parameters[0] = static_cast<unsigned char>(fileno);

		if (handleWriteData(DF_INS_LIMITED_CREDIT, parameters, sizeof(parameters), std::vector<unsigned char>(&value, &value + 4), mode))
		{
			ret = true;
		}
		
		return ret;
	}

	bool DESFireISO7816Commands::writeRecord(int fileno, size_t offset, size_t length, const void* data, EncryptionMode mode)
	{
		bool ret = false;

		unsigned char parameters[7];
		parameters[0] = static_cast<unsigned char>(fileno);
		parameters[1] = static_cast<unsigned char>(offset & 0xff);
		parameters[2] = static_cast<unsigned char>(static_cast<unsigned short>(offset & 0xff00) >> 8);
		parameters[3] = static_cast<unsigned char>(static_cast<unsigned int>(offset & 0xff0000) >> 16);
		parameters[4] = static_cast<unsigned char>(length & 0xff);
		parameters[5] = static_cast<unsigned char>(static_cast<unsigned short>(length & 0xff00) >> 8);
		parameters[6] = static_cast<unsigned char>(static_cast<unsigned int>(length & 0xff0000) >> 16);

		if (handleWriteData(DF_INS_WRITE_RECORD, parameters, sizeof(parameters), std::vector<unsigned char>((unsigned char*)data, (unsigned char*)data + length), mode))
		{
			ret = true;
		}
		
		return ret;
	}

	size_t DESFireISO7816Commands::readRecords(int fileno, size_t offset, size_t length, void* data, EncryptionMode mode)
	{
		unsigned char command[7];

		command[0] = static_cast<unsigned char>(fileno);
		command[1] = static_cast<unsigned char>(offset & 0xff);
		command[2] = static_cast<unsigned char>(static_cast<unsigned short>(offset & 0xff00) >> 8);
		command[3] = static_cast<unsigned char>(static_cast<unsigned int>(offset & 0xff0000) >> 16);
		command[4] = static_cast<unsigned char>(length & 0xff);
		command[5] = static_cast<unsigned char>(static_cast<unsigned short>(length & 0xff00) >> 8);
		command[6] = static_cast<unsigned char>(static_cast<unsigned int>(length & 0xff0000) >> 16);

		std::vector<unsigned char> result = transmit(DF_INS_READ_RECORDS, command, sizeof(command));
		unsigned char err = result.back();
		result.resize(result.size() - 2);
		result = handleReadData(err, result, length, mode);
		memcpy(data, &result[0], result.size());
		
		return result.size();
	}

	bool DESFireISO7816Commands::clearRecordFile(int fileno)
	{
		bool r = false;

		unsigned char command[1];
		command[0] = static_cast<unsigned char>(fileno);

		transmit(DF_INS_CLEAR_RECORD_FILE, command, sizeof(command));
		r = true;

		return r;
	}

	bool DESFireISO7816Commands::commitTransaction()
	{
		bool r = false;

		transmit(DF_COMMIT_TRANSACTION);
		r = true;

		return r;
	}

	bool DESFireISO7816Commands::abortTransaction()
	{
		bool r = false;
		
		transmit(DF_INS_ABORT_TRANSACTION);
		r = true;

		return r;
	}

	bool DESFireISO7816Commands::authenticate(unsigned char keyno)
	{
		boost::shared_ptr<DESFireKey> key = d_crypto->getKey(keyno);
		return authenticate(keyno, key);
	}

	bool DESFireISO7816Commands::authenticate(unsigned char keyno, boost::shared_ptr<DESFireKey> key)
	{
		unsigned char command[16];

		if (boost::dynamic_pointer_cast<SAMKeyStorage>(key->getKeyStorage()) && !getSAMChip())
			THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "SAMKeyStorage set on the key but not SAM reader has been set.");

		bool r = false;
		unsigned char diversify[16];
		if (d_crypto->getDiversify(diversify))
		{
			command[0] = keyno;
			std::vector<unsigned char> result = DESFireISO7816Commands::transmit(DF_INS_AUTHENTICATE, command, 1);
			if (result[result.size() - 1] == DF_INS_ADDITIONAL_FRAME && (result.size()-2) >= 8)
			{
				result.resize(8);
				std::vector<unsigned char> rndAB;

				if (boost::dynamic_pointer_cast<SAMKeyStorage>(key->getKeyStorage()))
				{
					boost::shared_ptr<SAMCommands> samcommands = boost::dynamic_pointer_cast<SAMCommands>(getSAMChip()->getCommands());
					boost::shared_ptr<ISO7816ReaderCardAdapter> readercardadapter = boost::dynamic_pointer_cast<ISO7816ReaderCardAdapter>(samcommands->getReaderCardAdapter());

					unsigned char apduresult[255];
					size_t apduresultlen = sizeof(apduresult);

					std::vector<unsigned char> data(2 + result.size());
					data[0] = keyno;
					data[1] = key->getKeyVersion();
					memcpy(&data[0] + 2, &result[0], result.size());

					readercardadapter->sendAPDUCommand(0x80, 0x0a, 0x02, 0x00, (unsigned char)(data.size()), &data[0], data.size(), 0x00, apduresult, &apduresultlen);
					if (apduresultlen <= 2)
						THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "sam authenticate DES P1 failed.");

					rndAB.insert(rndAB.begin(), apduresult, apduresult + 16);
				}
				else
					rndAB = d_crypto->authenticate_PICC1(keyno, diversify, result);

				result = DESFireISO7816Commands::transmit(DF_INS_ADDITIONAL_FRAME, &rndAB[0], 16);

				if ((result.size() - 2) >= 8)
				{
					result.resize(result.size() - 2);
					
					if (boost::dynamic_pointer_cast<SAMKeyStorage>(key->getKeyStorage()))
					{
						boost::shared_ptr<SAMCommands> samcommands = boost::dynamic_pointer_cast<SAMCommands>(getSAMChip()->getCommands());
						boost::shared_ptr<ISO7816ReaderCardAdapter> readercardadapter = boost::dynamic_pointer_cast<ISO7816ReaderCardAdapter>(samcommands->getReaderCardAdapter());

						unsigned char apduresult[255];
						size_t apduresultlen = sizeof(apduresult);

						apduresultlen = 255;
						readercardadapter->sendAPDUCommand(0x80, 0x0a, 0x00, 0x00, 0x08, &result[0], 0x08, apduresult, &apduresultlen);

						if (apduresultlen != 2 || apduresult[0] != 0x90 || apduresult[1] != 0x00)
							THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "sam authenticate DES P2 failed.");
						d_crypto->d_sessionKey = samcommands->dumpSessionKey();
						d_crypto->d_currentKeyNo = keyno;
					}
					else
						d_crypto->authenticate_PICC2(keyno, result);

					r = true;
				}
			}
		}
		return r;
	}

	std::vector<unsigned char> DESFireISO7816Commands::transmit(unsigned char cmd, unsigned char lc)
	{
		return transmit(cmd, NULL, lc, true);
	}

	std::vector<unsigned char> DESFireISO7816Commands::transmit(unsigned char cmd, const std::vector<unsigned char>& buf, bool forceLc)
	{		
		return transmit(cmd, &buf[0], buf.size(), forceLc);
	}

	std::vector<unsigned char> DESFireISO7816Commands::transmit(unsigned char cmd, const void* buf, size_t buflen, bool forceLc)
	{		
		unsigned char result[255];
		size_t resultlen = sizeof(result);

		if (buf != NULL)
		{
			getISO7816ReaderCardAdapter()->sendAPDUCommand(DF_CLA_ISO_WRAP, cmd, 0x00, 0x00, static_cast<unsigned char>(buflen), reinterpret_cast<const unsigned char*>(buf), buflen, 0x00, result, &resultlen);
		}
		else if (forceLc)
		{
			getISO7816ReaderCardAdapter()->sendAPDUCommand(DF_CLA_ISO_WRAP, cmd, 0x00, 0x00, static_cast<unsigned char>(buflen), 0x00, result, &resultlen);
		}
		else
		{
			getISO7816ReaderCardAdapter()->sendAPDUCommand(DF_CLA_ISO_WRAP, cmd, 0x00, 0x00, 0x00, result, &resultlen);
		}
		
		return std::vector<unsigned char>(result, result + resultlen);
	}
}
