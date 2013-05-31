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
		std::vector<unsigned char> command;
		std::vector<unsigned char> answer;
		command.push_back(static_cast<unsigned char>(Device::HID));
		command.push_back(static_cast<unsigned char>(HidCommand::COM));
		command.push_back(static_cast<unsigned char>(0x03));
		command.push_back(static_cast<unsigned char>('r'));
		command.push_back(static_cast<unsigned char>('b'));
		command.push_back(static_cast<unsigned char>(blockno));
		answer = getRplethReaderCardAdapter()->sendCommand (command, 0);
		memcpy(buf, &answer[0], answer.size());
		buflen = answer.size();
		res = buflen;
		return res;
	}
	
	size_t MifareRplethCommands::updateBinary(unsigned char blockno, const void* buf, size_t buflen)
	{
		size_t result = 0;
		return result;
	}
	
	// in progress
	bool MifareRplethCommands::loadKey(unsigned char keyno, MifareKeyType keytype, const void* key, size_t keylen, bool vol)
	{
		bool r = false;

		std::vector<unsigned char> command;
		std::vector<unsigned char> answer;
		command.push_back(static_cast<unsigned char>(Device::HID));
		command.push_back(static_cast<unsigned char>(HidCommand::COM));
		command.push_back(static_cast<unsigned char>(0x05));
		command.push_back(static_cast<unsigned char>('l'));
		command.push_back(static_cast<unsigned char>(keyno));
		command.push_back(static_cast<unsigned char>(keytype));
		command.push_back(static_cast<unsigned char>(keytype));
		command.push_back(static_cast<unsigned char>(keyno));
		answer = getRplethReaderCardAdapter()->sendCommand (command, 0);
		unsigned char result[256];
		size_t resultlen = 256;
		
		if (!vol && (result[resultlen - 2] == 0x63) && (result[resultlen - 1] == 0x86))
		{
			if (keyno == 0)
			{
				r = loadKey(keyno, keytype, key, keylen, true);
			}
		}
		else
		{
			r = true;
		}

		return r;
	}
	
	// in progress
	void MifareRplethCommands::loadKey(boost::shared_ptr<Location> location, boost::shared_ptr<Key> key, MifareKeyType keytype)
	{
		
	}
	
	// in progress
	bool MifareRplethCommands::authenticate(unsigned char blockno, unsigned char keyno, MifareKeyType keytype)
	{
		bool res = false;
		std::vector<unsigned char> command;
		std::vector<unsigned char> answer;
		command.push_back(static_cast<unsigned char>(Device::HID));
		command.push_back(static_cast<unsigned char>(HidCommand::COM));
		command.push_back(static_cast<unsigned char>(0x05));
		command.push_back(static_cast<unsigned char>('l'));
		command.push_back(static_cast<unsigned char>(blockno));
		command.push_back(static_cast<unsigned char>(keytype));
		command.push_back(static_cast<unsigned char>(keyno));
		answer = getRplethReaderCardAdapter()->sendCommand (command, 0);
		if (answer[0] == 'L')
			res = true;
		return res;
	}
	
	// in progress
	void MifareRplethCommands::authenticate(unsigned char blockno, boost::shared_ptr<KeyStorage> key_storage, MifareKeyType keytype)
	{
		std::vector<unsigned char> command;
		std::vector<unsigned char> answer;
		command.push_back(static_cast<unsigned char>(Device::HID));
		command.push_back(static_cast<unsigned char>(HidCommand::COM));
		command.push_back(static_cast<unsigned char>(0x00));
		command.push_back(static_cast<unsigned char>('l'));
		command.push_back(static_cast<unsigned char>(blockno));
		command.push_back(static_cast<unsigned char>(keytype));
		command.push_back(static_cast<unsigned char>(keytype));
		answer = getRplethReaderCardAdapter()->sendCommand (command, 0);
	}
}

