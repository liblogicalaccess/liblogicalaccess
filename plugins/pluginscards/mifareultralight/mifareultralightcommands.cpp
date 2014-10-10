/**
 * \file mifareultralightcommands.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Mifare Ultralight commands.
 */

#include "mifareultralightcommands.hpp"
#include "mifareultralightchip.hpp"

namespace logicalaccess
{
    boost::shared_ptr<MifareUltralightChip> MifareUltralightCommands::getMifareUltralightChip()
    {
        return boost::dynamic_pointer_cast<MifareUltralightChip>(getChip());
    }

    size_t MifareUltralightCommands::readPages(int start_page, int stop_page, void* buf, size_t buflen)
    {
        if (start_page > stop_page)
        {
            THROW_EXCEPTION_WITH_LOG(std::invalid_argument, "Start page can't be greater than stop page.");
        }

        size_t minsize = 0;
        size_t offset = 0;

        for (int i = start_page; i <= stop_page; ++i)
        {
            minsize += 4;
            if (buflen < minsize)
            {
                THROW_EXCEPTION_WITH_LOG(std::invalid_argument, "The buffer is too short.");
            }

            offset += readPage(i, reinterpret_cast<char*>(buf)+offset, 4);
        }

        return minsize;
    }

    size_t MifareUltralightCommands::writePages(int start_page, int stop_page, const void* buf, size_t buflen)
    {
        if (start_page > stop_page)
        {
            THROW_EXCEPTION_WITH_LOG(std::invalid_argument, "Start page can't be greater than stop page.");
        }

        size_t minsize = 0;
        size_t offset = 0;

        for (int i = start_page; i <= stop_page; ++i)
        {
            minsize += 4;
            if (buflen < minsize)
            {
                THROW_EXCEPTION_WITH_LOG(std::invalid_argument, "The buffer is too short.");
            }

            offset += writePage(i, reinterpret_cast<const char*>(buf)+offset, 4);
        }

        return minsize;
    }

    void MifareUltralightCommands::lockPage(int page)
    {
        unsigned char lockbits[4];
        memset(lockbits, 0x00, sizeof(lockbits));

        if (page >= 3 && page <= 7)
        {
            lockbits[2] |= static_cast<unsigned char>(1 << page);
        }
        else if (page >= 8 && page <= 15)
        {
            lockbits[3] |= static_cast<unsigned char>(1 << (page - 8));
        }

        writePage(2, lockbits, sizeof(lockbits));
    }
}