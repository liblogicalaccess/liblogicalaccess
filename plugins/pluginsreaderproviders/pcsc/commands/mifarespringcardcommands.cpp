/**
 * \file mifarespringcardcommands.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Mifare SpringCard commands.
 */

#include "../commands/mifarespringcardcommands.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>

#include "../pcscreaderprovider.hpp"
#include "mifarechip.hpp"

namespace logicalaccess
{
    MifareSpringCardCommands::MifareSpringCardCommands()
        : MifarePCSCCommands()
    {
    }

    MifareSpringCardCommands::~MifareSpringCardCommands()
    {
    }

    bool MifareSpringCardCommands::loadKey(unsigned char keyno, MifareKeyType keytype, const void* key, size_t keylen, bool vol)
    {
        bool r = false;

        std::vector<unsigned char> result, vector_key((unsigned char*)key, (unsigned char*)key + keylen);

        unsigned char keyindex = 0x00;
        if (keytype == KT_KEY_B)
        {
            keyindex = 0x10 + keyno;
        }

        result = getPCSCReaderCardAdapter()->sendAPDUCommand(0xFF, 0x82, 0x00, keyindex, static_cast<unsigned char>(vector_key.size()), vector_key);

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

    void MifareSpringCardCommands::authenticate(unsigned char blockno, unsigned char keyno, MifareKeyType keytype)
    {
        std::vector<unsigned char> command;

        unsigned char keyindex = 0x00 + keyno;
        if (keytype == KT_KEY_B)
        {
            keyindex = 0x10 + keyno;
        }

        command.push_back(0x01);
        command.push_back(0x00);
        command.push_back(blockno);
        command.push_back(0x00);
        command.push_back(keyindex);

        getPCSCReaderCardAdapter()->sendAPDUCommand(0xFF, 0x86, 0x00, 0x00, static_cast<unsigned char>(command.size()), command);
    }

	void MifareSpringCardCommands::restore(unsigned char blockno)
	{
		std::vector<unsigned char> buf;
		buf.push_back(0x00);
		buf.push_back(0x00);
		buf.push_back(0x00);
		buf.push_back(0x00);

		getPCSCReaderCardAdapter()->sendAPDUCommand(0xFF, 0xF5, 0xC2, blockno, static_cast<unsigned char>(buf.size()), buf);
	}

	void MifareSpringCardCommands::increment(unsigned char blockno, uint32_t value)
	{
		std::vector<unsigned char> buf;
		buf.push_back(static_cast<unsigned char>((value >> 24) & 0xff));
		buf.push_back(static_cast<unsigned char>((value >> 16) & 0xff));
		buf.push_back(static_cast<unsigned char>((value >> 8) & 0xff));
		buf.push_back(static_cast<unsigned char>(value & 0xff));

		getPCSCReaderCardAdapter()->sendAPDUCommand(0xFF, 0xF5, 0xC1, blockno, static_cast<unsigned char>(buf.size()), buf);
	}

	void MifareSpringCardCommands::decrement(unsigned char blockno, uint32_t value)
	{
		std::vector<unsigned char> buf;
		buf.push_back(static_cast<unsigned char>((value >> 24) & 0xff));
		buf.push_back(static_cast<unsigned char>((value >> 16) & 0xff));
		buf.push_back(static_cast<unsigned char>((value >> 8) & 0xff));
		buf.push_back(static_cast<unsigned char>(value & 0xff));

		getPCSCReaderCardAdapter()->sendAPDUCommand(0xFF, 0xF5, 0xC0, blockno, static_cast<unsigned char>(buf.size()), buf);
	}
}