/**
 * \file desfirerwk400commands.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Desfire commands for Rwk400 readers.
 */

#include "desfirerwk400commands.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>

#include "desfirechip.hpp"
#include "desfirecommands.hpp"
#include "logicalaccess/cards/computermemorykeystorage.hpp"
#include "logicalaccess/cards/readermemorykeystorage.hpp"
#include "logicalaccess/cards/samkeystorage.hpp"


#define UNUSED(x) (void)(x)

namespace logicalaccess
{
	DesfireRwk400Commands::DesfireRwk400Commands()
		: DESFireCommands()
	{

	}

	DesfireRwk400Commands::~DesfireRwk400Commands()
	{
		
	}

	DESFireCrypto& DesfireRwk400Commands::getCrypto()
	{
		return d_crypto;
	}

	void DesfireRwk400Commands::erase()
	{
		
	}
	
	void DesfireRwk400Commands::selectApplication(unsigned int aid)
	{
		bool res = true;
		std::vector<unsigned char> command;
		std::vector<unsigned char> answer;
		command.push_back(0x80);
		command.push_back(RWK400Commands::TRANSMIT);
		command.push_back(0xF7);
		command.push_back(0x03);
		command.push_back(0x06);
		command.push_back(0x0A);
		command.push_back(0x00);
		command.push_back(DF_INS_SELECT_APPLICATION);
		command.push_back(aid & 0xff);
		command.push_back(aid>>8 & 0xff);
		command.push_back(aid>>16 & 0xff);
		try
		{
			answer = getReaderCardAdapter()->sendCommand(command, 0);
		}
		catch(std::exception& ex)
		{
			INFO_SIMPLE_(ex.what());
		}
	}

	void DesfireRwk400Commands::createApplication(unsigned int aid, DESFireKeySettings settings, unsigned char maxNbKeys)
	{
		bool res = true;
		std::vector<unsigned char> command;
		std::vector<unsigned char> answer;
		command.push_back(0x80);
		command.push_back(RWK400Commands::TRANSMIT);
		command.push_back(0xF7);
		command.push_back(0x05);
		command.push_back(0x08);
		command.push_back(0x0A);
		command.push_back(0x00);
		command.push_back(DF_INS_CREATE_APPLICATION);
		command.push_back(aid & 0xff);
		command.push_back(aid>>8 & 0xff);
		command.push_back(aid>>16 & 0xff);
		command.push_back(static_cast<unsigned char>(settings));
		command.push_back(static_cast<unsigned char>(maxNbKeys));
		try
		{
			answer = getReaderCardAdapter()->sendCommand(command, 0);
		}
		catch(std::exception& ex)
		{
			INFO_SIMPLE_(ex.what());
		}
	}

	void DesfireRwk400Commands::deleteApplication(unsigned int aid)
	{
		UNUSED(aid);
	}

	std::vector<unsigned int> DesfireRwk400Commands::getApplicationIDs()
	{
		std::vector<unsigned int> res;
		unsigned int tmp = 0;
		std::vector<unsigned char> command;
		std::vector<unsigned char> answer;
		command.push_back(0x80);
		command.push_back(RWK400Commands::TRANSMIT);
		command.push_back(0xF7);
		command.push_back(0x59);
		command.push_back(0x03);
		command.push_back(0x0A);
		command.push_back(0x00);
		command.push_back(DF_INS_GET_APPLICATION_IDS);
		try
		{
			answer = getReaderCardAdapter()->sendCommand(command, 0);
			if (answer.size () > 5)
				answer.erase(answer.begin(), answer.begin()+3);
			if (answer.size() == 86)
			{
				for (unsigned int i = 0; i < answer.size(); i+=3)
				{	
					tmp = answer[i+2];
					tmp <<= 8;
					tmp |= answer[i+1];
					tmp <<= 8;
					tmp |= answer[i];
					if (tmp == 0)
						i = static_cast<unsigned int>(answer.size());
					else
					{
						res.push_back(tmp);
					}
				}
				if (res.size() > 0)
					res.erase(res.end()-1);
			}
		}
		catch(std::exception& ex)
		{
			INFO_SIMPLE_(ex.what());
		}
		return res;
	}

	void DesfireRwk400Commands::changeKey(unsigned char keyno, boost::shared_ptr<DESFireKey> key)
	{
		key->getData();
		UNUSED(keyno);
	}

	void DesfireRwk400Commands::getKeySettings(DESFireKeySettings& settings, unsigned char& maxNbKeys)
	{
	  UNUSED(settings);
	  UNUSED(maxNbKeys);
	}

	void DesfireRwk400Commands::changeKeySettings(DESFireKeySettings settings)
	{
		UNUSED(settings);
	}

	std::vector<unsigned char> DesfireRwk400Commands::getFileIDs()
	{
		std::vector<unsigned char> res;
		unsigned char tmp = 0;
		std::vector<unsigned char> command;
		std::vector<unsigned char> answer;
		command.push_back(0x80);
		command.push_back(RWK400Commands::TRANSMIT);
		command.push_back(0xF7);
		command.push_back(0x65);  // for 32 file * 3 bytes + 5 for internal answer
		command.push_back(0x03);
		command.push_back(0x0A);
		command.push_back(0x00);
		command.push_back(DF_INS_GET_FILE_IDS);
		try
		{
			answer = getReaderCardAdapter()->sendCommand(command, 0);
			if (answer.size () > 5)
				answer.erase(answer.begin(), answer.begin()+2);
			if (answer.size () % 3 == 0 && answer.size() > 0)
			{
				for (unsigned int i = 0; i < answer.size(); i+=3)
				{	
					tmp = answer[i+2];
					tmp <<= 8;
					tmp |= answer[i+1];
					tmp <<= 8;
					tmp |= answer[i];
					if (tmp == 0)
						i = static_cast<unsigned int>(answer.size());
					else
					{
						res.push_back(tmp);
					}
				}
				if (res.size() > 0)
					res.erase(res.end()-1);
			}
		}
		catch(std::exception& ex)
		{
			INFO_SIMPLE_(ex.what());
		}
		return res;
	}

	void DesfireRwk400Commands::getFileSettings(unsigned char fileno, DESFireCommands::FileSetting& fileSetting)
	{
		UNUSED(fileno);
		UNUSED(fileSetting);
	}

	void DesfireRwk400Commands::changeFileSettings(unsigned char fileno, EncryptionMode comSettings, DESFireAccessRights accessRights, bool plain)
	{
		UNUSED(fileno);
		UNUSED(comSettings);
		UNUSED(accessRights);
		UNUSED(plain);
	}

	void DesfireRwk400Commands::createStdDataFile(unsigned char fileno, EncryptionMode comSettings, DESFireAccessRights accessRights, unsigned int fileSize)
	{
		std::vector<unsigned char> command;
		std::vector<unsigned char> answer;

		try
		{
			command.push_back(0x80);
			command.push_back(RWK400Commands::TRANSMIT);
			command.push_back(0xF7);
			command.push_back(0x03);
			command.push_back(0x0A);
			command.push_back(0x0A);
			command.push_back(0x00);
			command.push_back(DF_INS_CREATE_STD_DATA_FILE);
			command.push_back(static_cast<unsigned char>(fileno));
			command.push_back(static_cast<unsigned char>(comSettings));
			command.push_back(static_cast<unsigned char>(accessRights.readAccess));
			command.push_back(static_cast<unsigned char>(accessRights.writeAccess));
			command.push_back(static_cast<unsigned char>(fileSize >> 16 & 0xff));
			command.push_back(static_cast<unsigned char>(fileSize >> 8 & 0xff));
			command.push_back(static_cast<unsigned char>(fileSize & 0xff));

			answer = getReaderCardAdapter()->sendCommand(command, 0);
		}
		catch(std::exception& ex)
		{
			INFO_SIMPLE_(ex.what());
		}
	}

	void DesfireRwk400Commands::createBackupFile(unsigned char fileno, EncryptionMode comSettings, DESFireAccessRights accessRights, unsigned int fileSize)
	{
		UNUSED(fileno);
		UNUSED(comSettings);
		UNUSED(accessRights);
		UNUSED(fileSize);
	}

	void DesfireRwk400Commands::createValueFile(unsigned char fileno, EncryptionMode comSettings, DESFireAccessRights accessRights, unsigned int lowerLimit, unsigned int upperLimit, unsigned int value, bool limitedCreditEnabled)
	{
		UNUSED(fileno);
		UNUSED(comSettings);
		UNUSED(accessRights);
		UNUSED(lowerLimit);
		UNUSED(upperLimit);
		UNUSED(value);
		UNUSED(limitedCreditEnabled);
	}

	void DesfireRwk400Commands::createLinearRecordFile(unsigned char fileno, EncryptionMode comSettings, DESFireAccessRights accessRights, unsigned int fileSize, unsigned int maxNumberOfRecords)
	{
		UNUSED(fileno);
		UNUSED(comSettings);
		UNUSED(accessRights);
		UNUSED(fileSize);
		UNUSED(maxNumberOfRecords);
	}

	void DesfireRwk400Commands::createCyclicRecordFile(unsigned char fileno, EncryptionMode comSettings, DESFireAccessRights accessRights, unsigned int fileSize, unsigned int maxNumberOfRecords)
	{
		UNUSED(fileno);
		UNUSED(comSettings);
		UNUSED(accessRights);
		UNUSED(fileSize);
		UNUSED(maxNumberOfRecords);
	}

	
	void DesfireRwk400Commands::deleteFile(unsigned char fileno)
	{
		UNUSED(fileno);
	}

	unsigned int DesfireRwk400Commands::readData(unsigned char fileno, unsigned int offset, unsigned int length, void* data, EncryptionMode mode)
	{
		UNUSED(mode);
		unsigned int res = 0;
		std::vector<unsigned char> command;
		std::vector<unsigned char> answer;
		command.push_back(0x80);
		command.push_back(RWK400Commands::TRANSMIT);
		command.push_back(0xF7);
		command.push_back(static_cast<unsigned char>(length)+5);
		command.push_back(0x0A);
		command.push_back(0x0A);
		command.push_back(0x00);
		command.push_back(DF_INS_READ_DATA);
		command.push_back(static_cast<unsigned char>(fileno));
		command.push_back(static_cast<unsigned char>(offset & 0xff));
		command.push_back(static_cast<unsigned char>(offset>>8 & 0xff));
		command.push_back(static_cast<unsigned char>(offset>>16 & 0xff));
		command.push_back(static_cast<unsigned char>(length));
		try
		{
			answer = getReaderCardAdapter()->sendCommand(command, 0);
			if (answer.size() == (length + 5))
			{
				answer.erase (answer.begin(), answer.begin()+5);
				memcpy(data, &answer[0], length);
			}
		}
		catch(std::exception& ex)
		{
			INFO_SIMPLE_(ex.what());
		}

		return res;
	}

	void DesfireRwk400Commands::writeData(unsigned char fileno, unsigned int offset, unsigned int length, const void* data, EncryptionMode mode)
	{
		UNUSED(mode);
		bool res = false;
		std::vector<unsigned char> command;
		std::vector<unsigned char> answer;
		command.push_back(0x80);
		command.push_back(RWK400Commands::TRANSMIT);
		command.push_back(0xF7);
		command.push_back(0x05);
		command.push_back(static_cast<unsigned char>(length)+0x0A);
		command.push_back(0x0A);
		command.push_back(0x00);
		command.push_back(DF_INS_WRITE_DATA);
		command.push_back(static_cast<unsigned char>(fileno));
		command.push_back(static_cast<unsigned char>(offset>>16 & 0xff));
		command.push_back(static_cast<unsigned char>(offset>>8 & 0xff));
		command.push_back(static_cast<unsigned char>(offset & 0xff));
		command.push_back(static_cast<unsigned char>(length));
		for (size_t i = 0; i < length; i++)
		{
			command.push_back(static_cast<unsigned char>(((char *)data)[i]));
		}
		try
		{
			answer = getReaderCardAdapter()->sendCommand(command, 0);
			res = true;
		}
		catch(std::exception& ex)
		{
			INFO_SIMPLE_(ex.what());
		}
	}

	void DesfireRwk400Commands::getValue(unsigned char fileno, EncryptionMode mode, unsigned int& value)
	{
		UNUSED(fileno);
		UNUSED(mode);
		UNUSED(value);
	}

	void DesfireRwk400Commands::credit(unsigned char fileno, unsigned int value, EncryptionMode mode)
	{
		UNUSED(fileno);
		UNUSED(value);
		UNUSED(mode);
	}

	void DesfireRwk400Commands::debit(unsigned char fileno, unsigned int value, EncryptionMode mode)
	{
		UNUSED(fileno);
		UNUSED(value);
		UNUSED(mode);
	}

	void DesfireRwk400Commands::limitedCredit(unsigned char fileno, unsigned int value, EncryptionMode mode) 
	{
		UNUSED(fileno);
		UNUSED(value);
		UNUSED(mode);
	}

	void DesfireRwk400Commands::writeRecord(unsigned char fileno, unsigned int offset, unsigned int length, const void* data, EncryptionMode mode)
	{
		UNUSED(fileno);
		UNUSED(offset);
		UNUSED(length);
		UNUSED(data);
		UNUSED(mode);
	}

	unsigned int DesfireRwk400Commands::readRecords(unsigned char fileno, unsigned int offset, unsigned int length, void* data, EncryptionMode mode)
	{
		unsigned int res = 0;
		UNUSED(fileno);
		UNUSED(offset);
		UNUSED(length);
		UNUSED(data);
		UNUSED(mode);
		return res;
	}

	void DesfireRwk400Commands::clearRecordFile(unsigned char fileno)
	{
		UNUSED(fileno);
	}

	void DesfireRwk400Commands::commitTransaction()
	{
	
	}

	void DesfireRwk400Commands::abortTransaction()
	{
		
	}

	void DesfireRwk400Commands::authenticate(unsigned char keyno)
	{
		boost::shared_ptr<DESFireKey> key = d_chip->getDESFireProfile()->getDefaultKey(DESFireKeyType::DF_KEY_DES);
		authenticate(keyno, key);
	}

	void DesfireRwk400Commands::authenticate(unsigned char keyno, boost::shared_ptr<DESFireKey> key)
	{
		if (d_chip)
		{
			try
			{
				std::vector<unsigned char> permutedKey = premutationKey(key->getData(), key->getLength());
				std::vector<unsigned char> answer;
				std::vector<unsigned char> command;
				std::vector<unsigned char> checksum;
				command.push_back(0x80);
				command.push_back(RWK400Commands::LOAD_KEY);
				command.push_back(0x00);
				command.push_back(keyno);
				command.push_back(0x0C);
				for (int i = 0; i < 8; i++)
				{
					command.push_back(0x00);
				}
				checksum = computeChecksum (command, permutedKey);
				command.erase(command.begin()+5, command.end());
				command.insert(command.end(), permutedKey.begin(), permutedKey.end());
				command.insert(command.end(), checksum.begin(), checksum.end());
				// load_key
				answer = getReaderCardAdapter()->sendCommand(command, 0);
				if (answer.size() > 1)
				{
					// select current key
					command.clear();
					command.push_back(0x80);
					command.push_back(RWK400Commands::SELECT_CURRENT_KEY);
					command.push_back(0x00);
					command.push_back(keyno+0x01);
					command.push_back(0x08);
					for (int i = 0; i < 8; i++)
					{
						command.push_back(0x00);
					}
					answer = getReaderCardAdapter()->sendCommand(command, 0);
					// select_card
					command.clear();
					command.push_back(0x80);
					command.push_back(RWK400Commands::SELECT_CARD);
					command.push_back(0x30);
					command.push_back(0x0A);
					command.push_back(0x09);
					answer = getReaderCardAdapter()->sendCommand(command, 0);
					// rats
					command.clear();
					command.push_back (0x80);
					command.push_back (RWK400Commands::TRANSMIT);
					command.push_back (0xF7);
					command.push_back (0x06);
					command.push_back (0x02);
					command.push_back (0xE0);
					command.push_back (0x80);
					answer = getReaderCardAdapter()->sendCommand(command, 0);
				}
			}
			catch(std::invalid_argument& ex)
			{
				INFO_SIMPLE_(ex.what());
			}
		}
	}


	void DesfireRwk400Commands::getVersion(DESFireCommands::DESFireCardVersion& dataVersion)
	{
		UNUSED(dataVersion);
	}

	void DesfireRwk400Commands::setChip(boost::shared_ptr<DESFireChip> chip)
	{
		d_chip = chip;
	}

	std::vector<unsigned char> DesfireRwk400Commands::premutationKey(const unsigned char * key, size_t keylength)
	{
		std::vector<unsigned char> res;
		unsigned char tmp = 0;
		if (keylength > 7)
		{
			for (int i = 7; i > 0; i--)
			{
				tmp = 0;
				for (int j = 7; j >= 0; j--)
				{
					tmp <<= 1;
					tmp |= key[j] >> i & 0x1;
				}
				res.push_back(tmp);
			}
			tmp = 0;
			for (unsigned int i = 0; i < res.size(); i++)
			{
				tmp ^= res[i];
			}
			res.push_back(static_cast<unsigned char>(~tmp));
		}
		return res;
	}

	std::vector<unsigned char> DesfireRwk400Commands::computeChecksum (std::vector<unsigned char> command, std::vector<unsigned char> permutedKey)
	{
		std::vector<unsigned char> res;
		std::vector<unsigned char> tmp;
		if (command.size() > 7 && permutedKey.size() > 7)
		{
			for (int i = 0; i < 8; i++)
			{
				tmp.push_back(command[i] ^ permutedKey[i]);
			}
			for (int i = 0; i < 4; i++)
			{
				res.push_back(tmp[i] ^ tmp[i+4]);
			}
		}
		return res;
	}
}

