/**
 * \file mifarepluscommands.cpp
 * \author Xavier SCHMERBER <xavier.schmerber@gmail.com>
 * \brief Commands for MifarePlus card.
 */

#include "mifarepluscommands.hpp"
#include "logicalaccess/myexception.hpp"
#include <stdlib.h>

namespace logicalaccess
{
    MifarePlusCommands::MifarePlusCommands() : d_crypto(std::shared_ptr<MifarePlusCrypto>(new MifarePlusCrypto()))
    {
    }

    MifarePlusCommands::~MifarePlusCommands()
    {
    }

    std::vector<unsigned char> MifarePlusCommands::LeftRotateSB(std::vector<unsigned char> string)
    {
        unsigned char retain;
        size_t size = string.size();
        size_t c = 1;

        retain = string[0];
        while (size > 1 && c < size)
        {
            string[c - 1] = string[c];
            c++;
        }
        string[c - 1] = retain;

        return (string);
    }

    std::vector<unsigned char> MifarePlusCommands::GetRandKey(size_t size)
    {
        std::vector<unsigned char> rand;

        rand.clear();
        rand.resize(size);
        if (RAND_bytes(&rand[0], static_cast<int>(rand.size())) != 1)
        {
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Cannot retrieve cryptographically strong bytes");
        }

        return (rand);
    }

    std::shared_ptr<MifarePlusCrypto> MifarePlusCommands::GetCrypto() const
    {
        return (d_crypto);
    }
}