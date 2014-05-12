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

		std::vector<unsigned char> result;

		result = getPCSCReaderCardAdapter()->sendAPDUCommand(0xFF, 0xB0, 0x00, static_cast<unsigned char>(page), 16);

		r = result.size() - 2;
		if (r > buflen)
		{
			r = buflen;
		}
		memcpy(buf, &result[0], r);

		return r;
	}

	size_t MifareUltralightPCSCCommands::writePage(int page, const void* buf, size_t buflen)
	{
		if ((buflen > 16) || (!buf))
		{
			THROW_EXCEPTION_WITH_LOG(std::invalid_argument, "Bad buffer parameter.");
		}

		size_t r = 0;

		std::vector<unsigned char> data;
		data.insert(data.begin(), (unsigned char*)buf, (unsigned char*)buf + buflen);

		std::vector<unsigned char> result;

		result = getPCSCReaderCardAdapter()->sendAPDUCommand(0xFF, 0xD6, 0x00, static_cast<unsigned char>(page), static_cast<unsigned char>(data.size()), data);
		r = buflen;

		return r;
	}	
}

