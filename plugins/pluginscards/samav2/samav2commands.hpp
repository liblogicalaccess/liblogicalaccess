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
    template <typename T, typename S>
    class LIBLOGICALACCESS_API SAMAV2Commands : public ICommands
    {
    public:
        virtual ByteVector dumpSecretKey(unsigned char keyno, unsigned char keyversion, ByteVector divInpu) = 0;

		virtual void activateOfflineKey(unsigned char keyno, unsigned char keyversion, ByteVector divInpu) = 0;

		virtual ByteVector decipherOfflineData(ByteVector data) = 0;

		virtual ByteVector encipherOfflineData(ByteVector data) = 0;
    };
}

#endif /* LOGICALACCESS_SAMAV2COMMANDS_HPP */