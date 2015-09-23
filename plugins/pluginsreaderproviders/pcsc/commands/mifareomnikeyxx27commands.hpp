/**
 * \file mifareomnikeyxx27commands.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Mifare omnikey xx27 card.
 */

#ifndef LOGICALACCESS_MIFAREOMNIKEYXX27COMMANDS_HPP
#define LOGICALACCESS_MIFAREOMNIKEYXX27COMMANDS_HPP

#include "mifarepcsccommands.hpp"
#include "../readercardadapters/pcscreadercardadapter.hpp"

#include <string>
#include <vector>
#include <iostream>

namespace logicalaccess
{
    /**
     * \brief The Mifare commands class for Omnikey xx27 reader.
     */
    class LIBLOGICALACCESS_API MifareOmnikeyXX27Commands : public virtual MifarePCSCCommands
    {
    public:

        /**
         * \brief Constructor.
         */
        MifareOmnikeyXX27Commands();

        /**
         * \brief Destructor.
         */
        virtual ~MifareOmnikeyXX27Commands();

    public:
        using MifarePCSCCommands::authenticate;
//        using MifarePCSCCommands::loadKey;

        /**
         * \brief Authenticate a block, given a key number.
         * \param blockno The block number.
         * \param keyno The key number, previously loaded with Mifare::loadKey().
         * \param keytype The key type.
         */
        void authenticate(unsigned char blockno, unsigned char keyno, MifareKeyType keytype);
    };
}

#endif /* LOGICALACCESS_MIFAREOMNIKEYXX27COMMANDS_HPP */