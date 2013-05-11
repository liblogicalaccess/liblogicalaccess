/**
 * \file MifareSCMCommands.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Mifare SCM commands.
 */

#include "../Commands/MifareSCMCommands.h"

#include <iostream>
#include <iomanip>
#include <sstream>

using std::endl;
using std::dec;
using std::hex;
using std::setfill;
using std::setw;
using std::ostringstream;
using std::istringstream;

#include "../PCSCReaderProvider.h"
#include "MifareChip.h"

namespace logicalaccess
{	
	MifareSCMCommands::MifareSCMCommands()
		: MifarePCSCCommands()
	{
		
	}

	MifareSCMCommands::~MifareSCMCommands()
	{
		
	}	

	bool MifareSCMCommands::loadKey(unsigned char keyno, MifareKeyType keytype, const void* key, size_t keylen, bool vol)
	{
		bool r = false;

		unsigned char result[256];
		size_t resultlen = 256;

		getPCSCReaderCardAdapter()->sendAPDUCommand(0xFF, 0x82, 0x00, static_cast<unsigned char>(keytype), static_cast<unsigned char>(keylen), reinterpret_cast<const unsigned char*>(key), keylen, result, &resultlen);

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

	bool MifareSCMCommands::authenticate(unsigned char blockno, unsigned char keyno, MifareKeyType keytype)
	{
		bool r = false;

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
		r = true;

		return r;
	}
}

