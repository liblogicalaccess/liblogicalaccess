/**
 * \file mifarecherrycommands.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Mifare Cherry commands.
 */

#include "../commands/mifarecherrycommands.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>

#include "../pcscreaderprovider.hpp"
#include "mifarechip.hpp"

namespace logicalaccess
{	
	MifareCherryCommands::MifareCherryCommands()
		: MifarePCSCCommands()
	{
		
	}

	MifareCherryCommands::~MifareCherryCommands()
	{
		
	}	

	bool MifareCherryCommands::loadKey(unsigned char keyno, MifareKeyType /*keytype*/, const void* key, size_t keylen, bool /*vol*/)
	{
		bool r = false;

		unsigned char result[256];
		size_t resultlen = 256;

		// To check on Cherry documentation why key #0 failed.
		if (keyno == 0)
		{
			keyno = 1;
		}

		getPCSCReaderCardAdapter()->sendAPDUCommand(0xFF, 0x82, 0x00, keyno, static_cast<unsigned char>(keylen), reinterpret_cast<const unsigned char*>(key), keylen, result, &resultlen);
		r = true;

		return r;
	}

	bool MifareCherryCommands::authenticate(unsigned char blockno, unsigned char keyno, MifareKeyType keytype)
	{
		bool r = false;

		unsigned char command[5];
		size_t commandlen = sizeof(command);

		// To check on Cherry documentation why key #0 failed.
		if (keyno == 0)
		{
			keyno = 1;
		}

		command[0] = 0x01;
		command[1] = 0x00;
		command[2] = blockno;
		command[3] = static_cast<unsigned char>(keytype);
		command[4] = keyno;

		unsigned char result[256];
		size_t resultlen = 256;

		getPCSCReaderCardAdapter()->sendAPDUCommand(0xFF, 0x86, 0x00, 0x00, static_cast<unsigned char>(commandlen), command, commandlen, result, &resultlen);
		r = true;

		return r;
	}
}

