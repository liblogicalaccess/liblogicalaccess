/**
 * \file mifareok5553commands.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Mifaire commands for OK5553 readers.
 */

#include "mifareok5553commands.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>

#include "mifarechip.hpp"
#include "logicalaccess/cards/computermemorykeystorage.hpp"
#include "logicalaccess/cards/readermemorykeystorage.hpp"
#include "logicalaccess/cards/samkeystorage.hpp"


namespace logicalaccess
{
	MifareOK5553Commands::MifareOK5553Commands()
		: MifareCommands()
	{

	}

	MifareOK5553Commands::~MifareOK5553Commands()
	{
		
	}	
	
	size_t MifareOK5553Commands::readBinary(unsigned char blockno, size_t len, void* buf, size_t buflen)
	{
		size_t res = 0;
		char tmp [3];
		std::vector<unsigned char> command;
		std::vector<unsigned char> answer;
		command.push_back(static_cast<unsigned char>('r'));
		command.push_back(static_cast<unsigned char>('b'));
		sprintf (tmp, "%.2X", blockno);
		command.push_back(static_cast<unsigned char>(tmp[0]));
		command.push_back(static_cast<unsigned char>(tmp[1]));
		answer = getOK5553ReaderCardAdapter()->sendCommand (command);
		// convert ascii in hexa
		answer = OK5553ReaderUnit::asciiToHex (answer);
		res = (len < answer.size()) ? len : answer.size();
		if (res <= buflen)
		{
			memcpy(buf, &answer[0], res);
		}
		else
		{
			res = 0;
		}

		return res;
	}
	
	size_t MifareOK5553Commands::updateBinary(unsigned char blockno, const void* buf, size_t buflen)
	{
		char tmp [3];
		std::vector<unsigned char> command;
		std::vector<unsigned char> answer;
		command.push_back(static_cast<unsigned char>('w'));
		command.push_back(static_cast<unsigned char>('b'));
		sprintf (tmp, "%.2X", blockno);
		command.push_back(static_cast<unsigned char>(tmp[0]));
		command.push_back(static_cast<unsigned char>(tmp[1]));
		for (size_t i = 0; i < buflen; i++)
		{
			sprintf (tmp, "%.2X", ((char *)buf)[i]);
			command.push_back(static_cast<unsigned char>(tmp[0]));
			command.push_back(static_cast<unsigned char>(tmp[1]));
		}
		answer = getOK5553ReaderCardAdapter()->sendCommand (command);
		if (answer.size () > 1)
			buflen = answer.size();
		else
			buflen = 0;
		return buflen;
	}
	
	bool MifareOK5553Commands::loadKey(unsigned char keyno, MifareKeyType /*keytype*/, const void* key, size_t keylen, bool vol)
	{
		bool r = true;
		if (!vol)
		{
			char buf [3];
			std::vector<unsigned char> command;
			std::vector<unsigned char> answer;
			command.push_back(static_cast<unsigned char>('w'));
			command.push_back(static_cast<unsigned char>('m'));
			sprintf (buf, "%.2X", keyno);
			command.push_back(static_cast<unsigned char>(buf[0]));
			command.push_back(static_cast<unsigned char>(buf[1]));
			for (size_t i = 0; i < keylen; i++)
			{
				sprintf (buf, "%.2X", ((char *)key)[i]);
				command.push_back(static_cast<unsigned char>(buf[0]));
				command.push_back(static_cast<unsigned char>(buf[1]));
			}
			answer = getOK5553ReaderCardAdapter()->sendCommand (command);
			if (answer.size () < 2)
				r = false;
		}
		return r;
	}
	
	void MifareOK5553Commands::loadKey(boost::shared_ptr<Location> location, boost::shared_ptr<Key> key, MifareKeyType keytype)
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
	
	void MifareOK5553Commands::authenticate(unsigned char blockno, unsigned char keyno, MifareKeyType keytype)
	{
		std::vector<unsigned char> command;
		std::vector<unsigned char> answer;
		char buf [3];
		command.push_back(static_cast<unsigned char>('l'));
		unsigned char sector = blockno / 4;
		sprintf (buf, "%.2X", sector);
		command.push_back(static_cast<unsigned char>(buf[0]));
		command.push_back(static_cast<unsigned char>(buf[1]));
		sprintf (buf, "%.2X", keyno);
		if (keytype == KT_KEY_A)
			buf [0] += 1;
		else
			buf [0] += 3;
		command.push_back(static_cast<unsigned char>(buf[0]));
		command.push_back(static_cast<unsigned char>(buf[1]));
		answer = getOK5553ReaderCardAdapter()->sendCommand (command);
		if (answer.size() > 0)
		{
			if (answer[0] == 'L')
			{
				THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Authentication error.");
			}
		}
	}
	
	// in progress
	void MifareOK5553Commands::authenticate(unsigned char blockno, boost::shared_ptr<KeyStorage> key_storage, MifareKeyType keytype)
	{
		if (boost::dynamic_pointer_cast<ComputerMemoryKeyStorage>(key_storage))
		{
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

