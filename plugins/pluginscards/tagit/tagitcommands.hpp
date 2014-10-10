/**
 * \file tagitcommands.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Tag-It commands.
 */

#ifndef LOGICALACCESS_TAGITCOMMANDS_HPP
#define LOGICALACCESS_TAGITCOMMANDS_HPP

#include "logicalaccess/key.hpp"
#include "../iso15693/iso15693commands.hpp"

namespace logicalaccess
{
    /**
     * \brief The Tag-It commands base class.
     */
    class LIBLOGICALACCESS_API TagItCommands
    {
    public:

        /**
         * \brief Get if a block is locked.
         * \param block The block number.
         * \return True if the block is locked, false otherwise.
         */
        static bool isLocked(boost::shared_ptr<ISO15693Commands> cmd, unsigned char block);
    };
}

#endif