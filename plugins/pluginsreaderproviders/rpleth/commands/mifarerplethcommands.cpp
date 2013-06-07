/**
 * \file mifarerplethcommands.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Mifaire commands for Rpleth readers.
 */

#include "mifarerplethcommands.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>

#include "mifarechip.hpp"
#include "logicalaccess/cards/computermemorykeystorage.hpp"
#include "logicalaccess/cards/readermemorykeystorage.hpp"
#include "logicalaccess/cards/samkeystorage.hpp"


namespace logicalaccess
{
	MifareRplethCommands::MifareRplethCommands()
		: MifareCommands()
	{

	}

	MifareRplethCommands::~MifareRplethCommands()
	{
		
	}	
	
	size_t MifareRplethCommands::readBinary(unsigned char blockno, size_t len, void* buf, size_t buflen)
	{
		size_t res = 0;
		char tmp [3];
		std::vector<unsigned char> command;
		std::vector<unsigned char> answer;
		std::vector<unsigned char> answer_hex;
		command.push_back(static_cast<unsigned char>(Device::HID));
		command.push_back(static_cast<unsigned char>(HidCommand::COM));
		command.push_back(static_cast<unsigned char>(0x04));
		command.push_back(static_cast<unsigned char>('r'));
		command.push_back(static_cast<unsigned char>('b'));
		sprintf (tmp, "%.2X", blockno);
		command.push_back(static_cast<unsigned char>(tmp[0]));
		command.push_back(static_cast<unsigned char>(tmp[1]));
		answer = getRplethReaderCardAdapter()->sendCommand (command, 0);
		// convert ascii in hexa
		for (size_t i = 0; i < answer.size ()-2; i+=2)
		{
			tmp [0] = answer[i];
			tmp [1] = answer[i+1];
			tmp [2] = '\0';
			answer_hex.push_back (static_cast<unsigned char>(strtoul (tmp, NULL, 16)));
		}
		res = (len < answer_hex.size()) ? len : answer_hex.size();
		if (res <= buflen)
		{
			memcpy(buf, &answer_hex[0], res);
		}
		else
		{
			res = 0;
		}

		return res;
	}
	
	size_t MifareRplethCommands::updateBinary(unsigned char blockno, const void* buf, size_t buflen)
	{
		size_t result = 0;
		char tmp [2];
		std::vector<unsigned char> command;
		std::vector<unsigned char> answer;
		command.push_back(static_cast<unsigned char>(Device::HID));
		command.push_back(static_cast<unsigned char>(HidCommand::COM));
		command.push_back(static_cast<unsigned char>(0x04+buflen));
		command.push_back(static_cast<unsigned char>('w'));
		command.push_back(static_cast<unsigned char>('b'));
		sprintf (tmp, "%.2X", blockno);
		command.push_back(static_cast<unsigned char>(tmp[0]));
		command.push_back(static_cast<unsigned char>(tmp[1]));
		for (size_t i = 0; i < buflen; i++)
		{
			command.push_back(static_cast<unsigned char>(((char *)buf)[i]));
		}
		answer = getRplethReaderCardAdapter()->sendCommand (command, 0);
		if (answer.size () > 1)
			buflen = answer.size();
		else
			buflen = 0;
		return result;
	}
	
	bool MifareRplethCommands::loadKey(unsigned char keyno, MifareKeyType /*keytype*/, const void* key, size_t keylen, bool vol)
	{
		bool r = true;
		if (!vol)
		{
			char buf [2];
			std::vector<unsigned char> command;
			std::vector<unsigned char> answer;
			command.push_back(static_cast<unsigned char>(Device::HID));
			command.push_back(static_cast<unsigned char>(HidCommand::COM));
			command.push_back(static_cast<unsigned char>(0x04+keylen));
			command.push_back(static_cast<unsigned char>('w'));
			command.push_back(static_cast<unsigned char>('m'));
			sprintf (buf, "%.2X", keyno);
			command.push_back(static_cast<unsigned char>(buf[0]));
			command.push_back(static_cast<unsigned char>(buf[1]));
			for (size_t i = 0; i < keylen; i++)
			{
				command.push_back(static_cast<unsigned char>(((char *)key)[i]));
			}
			answer = getRplethReaderCardAdapter()->sendCommand (command, 0);
			if (answer.size () < 2)
				r = false;
		}
		return r;
	}
	
	// in progress
	void MifareRplethCommands::loadKey(boost::shared_ptr<Location> location, boost::shared_ptr<Key> key, MifareKeyType keytype)
	{
		EXCEPTION_ASSERT_WITH_LOG(location, std::invalid_argument, "location cannot be null.");
		EXCEPTION_ASSERT_WITH_LOG(key, std::invalid_argument, "key cannot be null.");

		boost::shared_ptr<MifareLocation> mLocation = boost::dynamic_pointer_cast<MifareLocation>(location);
		boost::shared_ptr<MifareKey> mKey = boost::dynamic_pointer_cast<MifareKey>(key);
		
		EXCEPTION_ASSERT_WITH_LOG(mLocation, std::invalid_argument, "location must be a MifareLocation.");
		EXCEPTION_ASSERT_WITH_LOG(mKey, std::invalid_argument, "key must be a MifareKey.");
		
		boost::shared_ptr<KeyStorage> key_storage = key->getKeyStorage();

		if (boost::dynamic_pointer_cast<ComputerMemoryKeyStorage>(key_storage))
		{
			loadKey(0, keytype, key->getData(), key->getLength());
		}
		else if (boost::dynamic_pointer_cast<ReaderMemoryKeyStorage>(key_storage))
		{
			// Don't load the key when reader memory, except if specified
			if (!key->isEmpty())
			{
				boost::shared_ptr<ReaderMemoryKeyStorage> rmKs = boost::dynamic_pointer_cast<ReaderMemoryKeyStorage>(key_storage);
				loadKey(rmKs->getKeySlot(), keytype, key->getData(), key->getLength(), rmKs->getVolatile());
			}
		}
		else
		{
			THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "The key storage type is not supported for this card/reader.");
		}
	}
	
	bool MifareRplethCommands::authenticate(unsigned char blockno, unsigned char keyno, MifareKeyType keytype)
	{
		bool res = false;
		std::vector<unsigned char> command;
		std::vector<unsigned char> answer;
		char buf [2];
		command.push_back(static_cast<unsigned char>(Device::HID));
		command.push_back(static_cast<unsigned char>(HidCommand::COM));
		command.push_back(static_cast<unsigned char>(0x05));
		command.push_back(static_cast<unsigned char>('l'));
		unsigned char sector = blockno / 4;
		sprintf (buf, "%x%x", sector>>4, sector&0xf);
		command.push_back(static_cast<unsigned char>(buf[0]));
		command.push_back(static_cast<unsigned char>(buf[1]));
		sprintf (buf, "%x%x", keyno>>4, keyno&0xf);
		if (keytype == KT_KEY_A)
			buf [0] += 1;
		else
			buf [0] += 3;
		command.push_back(static_cast<unsigned char>(buf[0]));
		command.push_back(static_cast<unsigned char>(buf[1]));
		answer = getRplethReaderCardAdapter()->sendCommand (command, 0);
		if (answer.size() > 0)
			if (answer[0] == 'L')
				res = true;
		return res;
	}
	
	// in progress
	void MifareRplethCommands::authenticate(unsigned char blockno, boost::shared_ptr<KeyStorage> key_storage, MifareKeyType keytype)
	{
		if (boost::dynamic_pointer_cast<ComputerMemoryKeyStorage>(key_storage))
		{
			/*
			char tmp [3];
			sprintf (tmp, "%x%x", blockno>>4, blockno&0xf);
			tmp[2] = '\0';
			std::cout << "blockno : " << tmp << std::endl;
			*/
			authenticate(blockno, 0, keytype);
		}
		else if (boost::dynamic_pointer_cast<ReaderMemoryKeyStorage>(key_storage))
		{
			boost::shared_ptr<ReaderMemoryKeyStorage> rmKs = boost::dynamic_pointer_cast<ReaderMemoryKeyStorage>(key_storage);
			authenticate(blockno, rmKs->getKeySlot(), keytype);
		}
		else
		{
			THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "The key storage type is not supported for this card/reader.");
		}
	}
}

