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

	bool DesfireRwk400Commands::erase(bool resetKey)
	{
		bool res = false;

		return res;
	}
	
	bool DesfireRwk400Commands::selectApplication(int aid)
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
		catch(std::exception&)
		{
			res = false;
		}
		return res;
	}

	bool DesfireRwk400Commands::createApplication(int aid, DESFireKeySettings settings, int maxNbKeys)
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
		catch(std::exception&)
		{
			res = false;
		}
		return res;
	}

	bool DesfireRwk400Commands::deleteApplication(int aid)
	{
		bool res = false;

		return res;
	}

	std::vector<int> DesfireRwk400Commands::getApplicationIDs()
	{
		std::vector<int> res;
		int tmp = 0;
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
				for (int i = 0; i < answer.size(); i+=3)
				{	
					tmp = answer[i+2];
					tmp <<= 8;
					tmp |= answer[i+1];
					tmp <<= 8;
					tmp |= answer[i];
					if (tmp == 0)
						i = static_cast<int>(answer.size());
					else
					{
						res.push_back(tmp);
					}
				}
				if (res.size() > 0)
					res.erase(res.end()-1);
			}
		}
		catch(std::exception&)
		{
		}
		return res;
	}

	bool DesfireRwk400Commands::changeKey(unsigned char keyno, boost::shared_ptr<DESFireKey> key)
	{
		bool res = false;
		key->getData();
		return res;
	}

	void DesfireRwk400Commands::getKeySettings(DESFireKeySettings& settings, unsigned int& maxNbKeys)
	{
	}

	bool DesfireRwk400Commands::changeKeySettings(DESFireKeySettings settings)
	{
		bool res = false;
		return res;
	}

	std::vector<int> DesfireRwk400Commands::getFileIDs()
	{
		std::vector<int> res;
		int tmp = 0;
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
				for (int i = 0; i < answer.size(); i+=3)
				{	
					tmp = answer[i+2];
					tmp <<= 8;
					tmp |= answer[i+1];
					tmp <<= 8;
					tmp |= answer[i];
					if (tmp == 0)
						i = static_cast<int>(answer.size());
					else
					{
						res.push_back(tmp);
					}
				}
				if (res.size() > 0)
					res.erase(res.end()-1);
			}
		}
		catch(std::exception&)
		{
		}
		return res;
	}

	bool DesfireRwk400Commands::getFileSettings(int fileno, DESFireCommands::FileSetting& fileSetting)
	{
		bool res = false;

		return res;
	}

	bool DesfireRwk400Commands::changeFileSettings(int fileno, EncryptionMode comSettings, DESFireAccessRights accessRights, bool plain)
	{
		bool res = false;

		return res;
	}

	bool DesfireRwk400Commands::createStdDataFile(int fileno, EncryptionMode comSettings, DESFireAccessRights accessRights, int fileSize)
	{
		bool res = true;
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
		catch(std::exception&)
		{
			res = false;
		}
		return res;
	}

	bool DesfireRwk400Commands::createBackupFile(int fileno, EncryptionMode comSettings, DESFireAccessRights accessRights, int fileSize)
	{
		bool res = false;

		return res;
	}

	bool DesfireRwk400Commands::createValueFile(int fileno, EncryptionMode comSettings, DESFireAccessRights accessRights, unsigned int lowerLimit, unsigned int upperLimit, int value, bool limitedCreditEnabled)
	{
		bool res = false;

		return res;
	}

	bool DesfireRwk400Commands::createLinearRecordFile(int fileno, EncryptionMode comSettings, DESFireAccessRights accessRights, int fileSize, int maxNumberOfRecords)
	{
		bool res = false;

		return res;
	}

	bool DesfireRwk400Commands::createCyclicRecordFile(int fileno, EncryptionMode comSettings, DESFireAccessRights accessRights, int fileSize, int maxNumberOfRecords)
	{
		bool res = false;

		return res;
	}

	
	bool DesfireRwk400Commands::deleteFile(int fileno)
	{
		bool res = false;

		return res;
	}

	size_t DesfireRwk400Commands::readData(int fileno, size_t offset, size_t length, void* data, EncryptionMode mode)
	{
		size_t res = 0;
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
		catch(std::exception&)
		{
		}
		return res;
	}

	bool DesfireRwk400Commands::writeData(int fileno, size_t offset, size_t length, const void* data, EncryptionMode mode)
	{
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
		catch(std::exception&)
		{
			res = false;
		}
		return res;
	}

	bool DesfireRwk400Commands::getValue(int fileno, EncryptionMode mode, int& value)
	{
		bool res = false;

		return res;
	}

	bool DesfireRwk400Commands::credit(int fileno, int value, EncryptionMode mode)
	{
		bool res = false;

		return res;
	}

	bool DesfireRwk400Commands::debit(int fileno, int value, EncryptionMode mode)
	{
		bool res = false;

		return res;
	}

	bool DesfireRwk400Commands::limitedCredit(int fileno, int value, EncryptionMode mode) 
	{
		bool res = false;

		return res;
	}

	bool DesfireRwk400Commands::writeRecord(int fileno, size_t offset, size_t length, const void* data, EncryptionMode mode)
	{
		bool res = false;

		return res;
	}

	size_t DesfireRwk400Commands::readRecords(int fileno, size_t offset, size_t length, void* data, EncryptionMode mode)
	{
		size_t res = 0;

		return res;
	}

	bool DesfireRwk400Commands::clearRecordFile(int fileno)
	{
		bool res = false;

		return res;
	}

	bool DesfireRwk400Commands::commitTransaction()
	{
		bool res = false;

		return res;
	}

	bool DesfireRwk400Commands::abortTransaction()
	{
		bool res = false;

		return res;
	}

	bool DesfireRwk400Commands::authenticate(unsigned char keyno)
	{
		bool res = false;
		if (d_chip)
		{
			try
			{
				boost::shared_ptr<DESFireKey> key = d_chip->getDESFireProfile()->getDefaultKey(DESFireKeyType::DF_KEY_DES);
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
					res = true;
				}
			}
			catch(std::invalid_argument&)
			{

			}
		}
		return res;
	}


	bool DesfireRwk400Commands::getVersion(DESFireCommands::DESFireCardVersion& dataVersion)
	{
		bool res = false;

		return res;
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
			for (int i = 0; i < res.size(); i++)
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

