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
		size_t res;
		std::vector<unsigned char> command;
		std::vector<unsigned char> answer;
		char buffer [3];
		command.push_back(static_cast<unsigned char>('r'));
		command.push_back(static_cast<unsigned char>('b'));
		sprintf (buffer, "%.2X", page);
		command.push_back(static_cast<unsigned char>(buffer[0]));
		command.push_back(static_cast<unsigned char>(buffer[1]));
		answer = getRplethReaderCardAdapter()->sendCommand (command);
		answer = RplethReaderUnit::asciiToHex(answer);
		res = (buflen <= answer.size()) ?  buflen : answer.size();
		memcpy(buf, &answer[0], res);
		return res;
	}
	
	size_t MifareUltralightRplethCommands::writePage(int page, const void* buf, size_t buflen)
	{
		size_t res;
		std::vector<unsigned char> command;
		std::vector<unsigned char> answer;
		char buffer [3];
		command.push_back(static_cast<unsigned char>('w'));
		command.push_back(static_cast<unsigned char>('b'));
		sprintf (buffer, "%.2X", page);
		command.push_back(static_cast<unsigned char>(buffer[0]));
		command.push_back(static_cast<unsigned char>(buffer[1]));
		for (size_t i = 0; i < buflen; i++)
		{
			sprintf (buffer, "%.2X", ((char*)buf)[i]);
			command.push_back(static_cast<unsigned char>(buffer[0]));
			command.push_back(static_cast<unsigned char>(buffer[1]));
		}
		answer = getRplethReaderCardAdapter()->sendCommand (command);
		res = buflen;
		return res;
	}
}

