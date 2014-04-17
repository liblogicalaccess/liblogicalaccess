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

		// To check on Cherry documentation why key #0 failed.
		if (keyno == 0)
		{
			keyno = 1;
		}
		std::vector<unsigned char> vector_key((unsigned char*)key, (unsigned char*)key + keylen);
		getPCSCReaderCardAdapter()->sendAPDUCommand(0xFF, 0x82, 0x00, keyno, static_cast<unsigned char>(keylen), vector_key);
		r = true;

		return r;
	}

	void MifareCherryCommands::authenticate(unsigned char blockno, unsigned char keyno, MifareKeyType keytype)
	{
		std::vector<unsigned char> command;

		// To check on Cherry documentation why key #0 failed.
		if (keyno == 0)
		{
			keyno = 1;
		}

		command.push_back(0x01);
		command.push_back(0x00);
		command.push_back(blockno);
		command.push_back(static_cast<unsigned char>(keytype));
		command.push_back(keyno);

		getPCSCReaderCardAdapter()->sendAPDUCommand(0xFF, 0x86, 0x00, 0x00, static_cast<unsigned char>(command.size()), command);
	}
}

