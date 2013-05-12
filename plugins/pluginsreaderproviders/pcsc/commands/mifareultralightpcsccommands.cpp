/**
 * \file mifareultralightpcsccommands.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Mifare Ultralight PC/SC commands.
 */

#include "../commands/mifareultralightpcsccommands.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>

#include "../pcscreaderprovider.hpp"
#include "mifareultralightchip.hpp"
#include "logicalaccess/cards/computermemorykeystorage.hpp"
#include "logicalaccess/cards/readermemorykeystorage.hpp"
#include "logicalaccess/cards/samkeystorage.hpp"

namespace logicalaccess
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

