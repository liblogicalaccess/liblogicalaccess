/**
 * \file MifareUltralightPCSCCommands.cpp
 * \author Maxime CHAMLEY <maxime.chamley@islog.eu>
 * \brief Mifare Ultralight PC/SC commands.
 */

#include "../Commands/MifareUltralightPCSCCommands.h"

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
#include "MifareUltralightChip.h"
#include "logicalaccess/Cards/ComputerMemoryKeyStorage.h"
#include "logicalaccess/Cards/ReaderMemoryKeyStorage.h"
#include "logicalaccess/Cards/SAMKeyStorage.h"

namespace LOGICALACCESS
{	
	MifareUltralightPCSCCommands::MifareUltralightPCSCCommands()
		: MifareUltralightCommands()
	{
		
	}

	MifareUltralightPCSCCommands::~MifareUltralightPCSCCommands()
	{
		
	}

	size_t MifareUltralightPCSCCommands::readPage(int page, void* buf, size_t buflen)
	{
		if ((buflen >= 16) || (!buf))
		{
			THROW_EXCEPTION_WITH_LOG(std::invalid_argument, "Bad buffer parameter.");
		}

		size_t r = 0;

		unsigned char result[256];
		size_t resultlen = 256;

		getPCSCReaderCardAdapter()->sendAPDUCommand(0xFF, 0xB0, 0x00, static_cast<unsigned char>(page), 16, result, &resultlen);

		r = resultlen - 2;
		if (r > buflen)
		{
			r = buflen;
		}
		memcpy(buf, result, r);

		return r;
	}

	size_t MifareUltralightPCSCCommands::writePage(int page, const void* buf, size_t buflen)
	{
		if ((buflen > 16) || (!buf))
		{
			THROW_EXCEPTION_WITH_LOG(std::invalid_argument, "Bad buffer parameter.");
		}

		size_t r = 0;

		unsigned char data[16];
		memset(data, 0x00, sizeof(data));
		memcpy(data, buf, buflen);

		unsigned char result[256];
		size_t resultlen = sizeof(result);

		getPCSCReaderCardAdapter()->sendAPDUCommand(0xFF, 0xD6, 0x00, static_cast<unsigned char>(page), sizeof(data), data, sizeof(data), result, &resultlen);
		r = buflen;

		return r;
	}	
}

