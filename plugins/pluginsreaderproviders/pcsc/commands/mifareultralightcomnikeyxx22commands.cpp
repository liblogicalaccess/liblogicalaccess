/**
* \file mifareultralightcomnikeyxx22commands.cpp
* \author Maxime C. <maxime-dev@islog.com>
* \brief Mifare Ultralight C - Omnikey xx22.
*/

#include "../commands/mifareultralightcomnikeyxx22commands.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>

#include "../pcscreaderprovider.hpp"
#include "logicalaccess/cards/computermemorykeystorage.hpp"
#include "logicalaccess/cards/readermemorykeystorage.hpp"
#include "logicalaccess/cards/samkeystorage.hpp"

namespace logicalaccess
{
    MifareUltralightCOmnikeyXX22Commands::MifareUltralightCOmnikeyXX22Commands()
        : MifareUltralightCCommands(CMD_MIFAREULTRALIGHTCOMNIKEYXX22)
    {
		bridge = std::make_unique<MifareUltralightPCSCCommands>(CMD_MIFAREULTRALIGHTCOMNIKEYXX22);
    }

	MifareUltralightCOmnikeyXX22Commands::MifareUltralightCOmnikeyXX22Commands(std::string ct)
		: MifareUltralightCCommands(ct)
	{
		bridge = std::make_unique<MifareUltralightPCSCCommands>(ct);
	}

    MifareUltralightCOmnikeyXX22Commands::~MifareUltralightCOmnikeyXX22Commands()
    {
    }

    void MifareUltralightCOmnikeyXX22Commands::authenticate(std::shared_ptr<TripleDESKey> authkey)
    {
        if (!authkey)
        {
            authkey.reset(new TripleDESKey("00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00"));
        }

        EXCEPTION_ASSERT(authkey->getLength() >= 16, LibLogicalAccessException, "Invalid key length.");

        // Load key part1 to key slot 1
        getPCSCReaderCardAdapter()->sendAPDUCommand(0xFF, 0x82, 0x00, 0x00, 0x08, std::vector<unsigned char>(authkey->getData(), authkey->getData() + 8));
        // Load key part2 to key slot 2
        getPCSCReaderCardAdapter()->sendAPDUCommand(0xFF, 0x82, 0x00, 0x01, 0x08, std::vector<unsigned char>(authkey->getData() + 8, authkey->getData() + 16));

        // Authenticate with key 0
        std::vector<unsigned char> command;
        command.push_back(0x01);
        command.push_back(0x00);
        command.push_back(0x01);
        command.push_back(0x60);
        command.push_back(0x00);
        getPCSCReaderCardAdapter()->sendAPDUCommand(0xFF, 0x86, 0x00, 0x00, static_cast<unsigned char>(command.size()), command);
    }
}