/**
 * \file mifareultralightcommands.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Mifare Ultralight commands.
 */

#include <logicalaccess/logs.hpp>
#include "mifareultralightcommands.hpp"
#include "mifareultralightchip.hpp"

namespace logicalaccess
{
    std::shared_ptr<MifareUltralightChip> MifareUltralightCommands::getMifareUltralightChip()
    {
        return std::dynamic_pointer_cast<MifareUltralightChip>(getChip());
    }

    ByteVector MifareUltralightCommands::readPages(int start_page, int stop_page)
    {
		ByteVector ret;

        if (start_page > stop_page)
        {
            THROW_EXCEPTION_WITH_LOG(std::invalid_argument, "Start page can't be greater than stop page.");
        }

        for (int i = start_page; i <= stop_page; ++i)
        {
            ByteVector data = readPage(i);
            // Some commands implementation returns more than one block (eg. PC/SC)
            if (data.size() > 4)
            {
                i += static_cast<int>((data.size() + 3) / 4) - 1;
            }
			ret.insert(ret.end(), data.begin(), data.end());
        }

        return ret;
    }

    void MifareUltralightCommands::writePages(int start_page, int stop_page, const ByteVector& buf)
    {
        if (start_page > stop_page)
        {
            THROW_EXCEPTION_WITH_LOG(std::invalid_argument, "Start page can't be greater than stop page.");
        }

        size_t offset = 0;
        for (int i = start_page; i <= stop_page; ++i)
        {
			ByteVector tmp(buf.begin() + offset, buf.begin() + offset + 4);
			writePage(i, tmp);
            offset += 4;
        }
    }

    void MifareUltralightCommands::lockPage(int page)
    {
        ByteVector lockbits(4, 0x00);

        if (page >= 3 && page <= 7)
        {
            lockbits[2] |= static_cast<unsigned char>(1 << page);
        }
        else if (page >= 8 && page <= 15)
        {
            lockbits[3] |= static_cast<unsigned char>(1 << (page - 8));
        }

        writePage(2, lockbits);
    }
}