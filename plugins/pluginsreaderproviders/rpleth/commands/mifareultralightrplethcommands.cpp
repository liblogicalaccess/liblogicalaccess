/**
 * \file mifareultralightrplethcommands.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Mifare Ultralight commands for Rpleth readers.
 */

#include "../commands/mifareultralightrplethcommands.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>

#include "mifarechip.hpp"


namespace logicalaccess
{
	MifareUltralightRplethCommands::MifareUltralightRplethCommands()
		: MifareUltralightCommands()
	{

	}

	MifareUltralightRplethCommands::~MifareUltralightRplethCommands()
	{
		
	}	
	
	size_t MifareUltralightRplethCommands::readPage(int page, void* buf, size_t buflen)
	{
		size_t result;
		std::vector<unsigned char> command;
		std::vector<unsigned char> answer;
		command.push_back(static_cast<unsigned char>(Device::HID));
		command.push_back(static_cast<unsigned char>(HidCommand::COM));
		command.push_back(static_cast<unsigned char>(0x03));
		command.push_back(static_cast<unsigned char>('r'));
		command.push_back(static_cast<unsigned char>('b'));
		command.push_back(static_cast<unsigned char>(page));
		answer = getRplethReaderCardAdapter()->sendCommand (command, 0);
		memcpy(buf, &answer[0], answer.size());
		result = answer.size();
		return result;
	}
	
	size_t MifareUltralightRplethCommands::writePage(int page, const void* buf, size_t buflen)
	{
		size_t result;
		std::vector<unsigned char> command;
		std::vector<unsigned char> answer;
		command.push_back(static_cast<unsigned char>(Device::HID));
		command.push_back(static_cast<unsigned char>(HidCommand::COM));
		command.push_back(static_cast<unsigned char>(buflen+0x03));
		command.push_back(static_cast<unsigned char>('w'));
		command.push_back(static_cast<unsigned char>('b'));
		command.push_back(static_cast<unsigned char>(page));
		for (size_t i = 0; i < buflen; i++)
		{
			command.push_back(static_cast<unsigned char>(((char*)buf)[i]));
		}
		answer = getRplethReaderCardAdapter()->sendCommand (command, 0);
		result = answer.size();
		return result;
	}
}

