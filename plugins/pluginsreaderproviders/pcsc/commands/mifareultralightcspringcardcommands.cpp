/**
 * \file mifareultralightcspringcardcommands.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Mifare Ultralight C - SpringCard.
 */

#include "../commands/mifareultralightcspringcardcommands.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>

#include "../pcscreaderprovider.hpp"
#include "logicalaccess/cards/computermemorykeystorage.hpp"
#include "logicalaccess/cards/readermemorykeystorage.hpp"
#include "logicalaccess/cards/samkeystorage.hpp"


namespace logicalaccess
{	
	MifareUltralightCSpringCardCommands::MifareUltralightCSpringCardCommands()
		: MifareUltralightCPCSCCommands()
	{
		
	}

	MifareUltralightCSpringCardCommands::~MifareUltralightCSpringCardCommands()
	{
		
	}

    std::vector<unsigned char> MifareUltralightCSpringCardCommands::sendGenericCommand(const std::vector<unsigned char>& data)
    {
        return getPCSCReaderCardAdapter()->sendAPDUCommand(0xFF, 0xFE, 0x01, 0x08, static_cast<unsigned char>(data.size()), data);
    }
}

