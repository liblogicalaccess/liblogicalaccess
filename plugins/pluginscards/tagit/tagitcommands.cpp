/**
 * \file tagitcommands.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Tag-It commands.
 */

#include "tagitcommands.hpp"

namespace logicalaccess
{
    bool TagItCommands::isLocked(std::shared_ptr<ISO15693Commands> cmd, unsigned char block)
    {
        unsigned char status = cmd->getSecurityStatus(block);

        return ((status & 0x01) != 0);
    }
}