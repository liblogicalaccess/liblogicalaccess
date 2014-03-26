/**
 * \file mifarescmcommands.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Mifare SCM commands.
 */

#include "../commands/mifareomnikeyxx27commands.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>

#include "../pcscreaderprovider.hpp"
#include "mifarechip.hpp"

namespace logicalaccess
{	
	MifareOmnikeyXX27Commands::MifareOmnikeyXX27Commands()
		: MifarePCSCCommands()
	{
		
	}

	MifareOmnikeyXX27Commands::~MifareOmnikeyXX27Commands()
	{
		
	}

	void MifareOmnikeyXX27Commands::authenticate(unsigned char blockno, unsigned char keyno, MifareKeyType keytype)
	{
		unsigned char command[5];
		size_t commandlen = sizeof(command);

		command[0] = 0x01;
		command[1] = 0x00;
		command[2] = blockno;
		command[3] = static_cast<unsigned char>(keytype);
		command[4] = keyno;

		unsigned char result[256];
		size_t resultlen = 256;

		getPCSCReaderCardAdapter()->sendAPDUCommand(0xFF, 0x86, 0x00, 0x00, static_cast<unsigned char>(commandlen), command, commandlen, result, &resultlen);
	}
}

