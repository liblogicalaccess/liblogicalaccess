/**
 * \file topazcommands.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Topaz commands.
 */

#include <logicalaccess/logs.hpp>
#include "topazcommands.hpp"
#include "topazchip.hpp"

namespace logicalaccess
{
    std::shared_ptr<TopazChip> TopazCommands::getTopazChip() const
    {
        return std::dynamic_pointer_cast<TopazChip>(getChip());
    }

    ByteVector TopazCommands::readPages(int start_page, int stop_page)
    {
		ByteVector ret;

        if (start_page > stop_page)
        {
            THROW_EXCEPTION_WITH_LOG(std::invalid_argument, "Start page can't be greater than stop page.");
        }

        for (int i = start_page; i <= stop_page; ++i)
        {
            ByteVector data = readPage(i);
			ret.insert(ret.end(), data.begin(), data.end());
        }

        return ret;
    }

    void TopazCommands::writePages(int start_page, int stop_page, const ByteVector& buf)
    {
        if (start_page > stop_page)
        {
            THROW_EXCEPTION_WITH_LOG(std::invalid_argument, "Start page can't be greater than stop page.");
        }

        size_t offset = 0;
        for (int i = start_page; i <= stop_page; ++i)
        {
			ByteVector tmp(buf.begin() + offset, buf.begin() + offset + 8);
			writePage(i, tmp);
            offset += 8;
        }
    }

    void TopazCommands::lockPage(int /*page*/)
    {
        // TODO: support it

        // Block 0xE: LOCK0 + LOCK1 + OTP0 + OTP1 + OTP2 + OTP3 + OTP4 + OTP5
        // If Topaz512, Block 0xF: OTP6 + OTP7 + LOCK2 + LOCK3 + LOCK4 + LOCK5 + LOCK6 + LOCK7
    }
}