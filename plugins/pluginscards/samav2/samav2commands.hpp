/**
 * \file SAMCommands.hpp
 * \author Adrien J. <adrien.jund@islog.com>
 * \brief SAMAV2Commands commands.
 */

#ifndef LOGICALACCESS_SAMAV2COMMANDS_HPP
#define LOGICALACCESS_SAMAV2COMMANDS_HPP

#include "samcommands.hpp"

namespace logicalaccess
{
#define CMD_SAMAV2	"SAMAV2"

    template <typename T, typename S>
    class LIBLOGICALACCESS_API SAMAV2Commands : public SAMCommands < T, S >
    {
    public:
		SAMAV2Commands() : SAMCommands<T, S>(CMD_SAMAV2) {}

		SAMAV2Commands(std::string ct) : SAMCommands<T, S>(ct) {}

        virtual std::vector<unsigned char> dumpSecretKey(unsigned char keyno, unsigned char keyversion, std::vector<unsigned char> divInpu) = 0;
    };
}

#endif /* LOGICALACCESS_SAMAV2COMMANDS_HPP */