/**
 * \file mifarescmcommands.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Mifare SCM commands.
 */

#include "../commands/mifarescmcommands.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>

#include "../pcscreaderprovider.hpp"
#include "mifarechip.hpp"

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

        std::vector<unsigned char> result, vector_key((unsigned char*)key, (unsigned char*)key + keylen);

        result = getPCSCReaderCardAdapter()->sendAPDUCommand(0xFF, 0x82, 0x00, static_cast<unsigned char>(keytype), static_cast<unsigned char>(vector_key.size()), vector_key);

        if (!vol && (result[result.size() - 2] == 0x63) && (result[result.size() - 1] == 0x86))
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

    void MifareSCMCommands::authenticate(unsigned char blockno, unsigned char keyno, MifareKeyType keytype)
    {
        std::vector<unsigned char> command;

        command.push_back(0x01);
        command.push_back(0x00);
        command.push_back(blockno);
        command.push_back(static_cast<unsigned char>(keytype));
        command.push_back(keyno);

        getPCSCReaderCardAdapter()->sendAPDUCommand(0xFF, 0x86, 0x00, 0x00, static_cast<unsigned char>(command.size()), command);
    }

	void MifareSCMCommands::increment(unsigned char blockno, uint32_t value)
	{
		std::vector<unsigned char> buf;
		buf.push_back(0xC0);
		buf.push_back(blockno);
		buf.push_back(static_cast<unsigned char>(value & 0xff));
		buf.push_back(static_cast<unsigned char>((value >> 8) & 0xff));
		buf.push_back(static_cast<unsigned char>((value >> 16) & 0xff));
		buf.push_back(static_cast<unsigned char>((value >> 24) & 0xff));

		getPCSCReaderCardAdapter()->sendAPDUCommand(0xFF, 0xF0, 0x00, blockno, static_cast<unsigned char>(buf.size()), buf);
	}

	void MifareSCMCommands::decrement(unsigned char blockno, uint32_t value)
	{
		std::vector<unsigned char> buf;
		buf.push_back(0xC1);
		buf.push_back(blockno);
		buf.push_back(static_cast<unsigned char>(value & 0xff));
		buf.push_back(static_cast<unsigned char>((value >> 8) & 0xff));
		buf.push_back(static_cast<unsigned char>((value >> 16) & 0xff));
		buf.push_back(static_cast<unsigned char>((value >> 24) & 0xff));

		getPCSCReaderCardAdapter()->sendAPDUCommand(0xFF, 0xF0, 0x00, blockno, static_cast<unsigned char>(buf.size()), buf);
	}
}