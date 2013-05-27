/**
 * \file mifarerplethcommands.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Mifaire commands for Rpleth readers.
 */

#include "../commands/mifarerplethcommands.hpp"

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
		answer = getDefaultRplethReaderCardAdapter()->sendCommand (command, 0);
		for (int i = 0; i < 16-len; i++)
		{
			answer.pop_back();
		}
		buflen = answer.size();
		char * tmp = malloc(buflen * sizeof(char));
		for (int i = 0; i < answer.size(); i++)
		{
			tmp[i] = answer[i];
		}
		memcpy (buf, tmp, buflen);
		free (tmp);
		res = buflen;
		return res;
	}
	
	size_t MifareRplethCommands::updateBinary(unsigned char blockno, const void* buf, size_t buflen)
	{
		
	}
	
	bool MifareRplethCommands::loadKey(unsigned char keyno, MifareKeyType keytype, const void* key, size_t keylen, bool vol = false)
	{
		bool r = false;

		std::vector<unsigned char> command;
		std::vector<unsigned char> answer;
		command.push_back(static_cast<unsigned char>(Device::HID));
		command.push_back(static_cast<unsigned char>(HidCommand::COM));
		command.push_back(static_cast<unsigned char>(0x05));
		command.push_back(static_cast<unsigned char>('l'));
		command.push_back(static_cast<unsigned char>(blockno));
		command.push_back(static_cast<unsigned char>(keytype));
		command.push_back(static_cast<unsigned char>(keytype));
		command.push_back(static_cast<unsigned char>(keyno));
		answer = getDefaultRplethReaderCardAdapter()->sendCommand (command, 0);
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
	
	void MifareRplethCommands::loadKey(boost::shared_ptr<Location> location, boost::shared_ptr<Key> key, MifareKeyType keytype)
	{
		
	}
	
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
		answer = getDefaultRplethReaderCardAdapter()->sendCommand (command, 0);
		if (answer[0] == 'L')
			res = true;
		return res;
	}
	
	void MifareRplethCommands::authenticate(unsigned char blockno, boost::shared_ptr<KeyStorage> key_storage, MifareKeyType keytype)
	{
		bool res = false;
		std::vector<unsigned char> command;
		std::vector<unsigned char> answer;
		command.push_back(static_cast<unsigned char>(Device::HID));
		command.push_back(static_cast<unsigned char>(HidCommand::COM));
		command.push_back(static_cast<unsigned char>(0x00));
		command.push_back(static_cast<unsigned char>('l'));
		command.push_back(static_cast<unsigned char>(blockno));
		command.push_back(static_cast<unsigned char>(keytype));
		command.push_back(static_cast<unsigned char>(keytype));
		command.push_back(static_cast<unsigned char>(keyno));
		answer = getDefaultRplethReaderCardAdapter()->sendCommand (command, 0);
		if (answer[0] == 'L')
			res = true;
		return res;
	}
}

