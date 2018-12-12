/**
 * \file epassiso7816commands.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief EPass ISO7816 commands.
 */

#include <logicalaccess/plugins/readers/iso7816/commands/epassiso7816commands.hpp>

namespace logicalaccess
{
EPassISO7816Commands::EPassISO7816Commands()
    : EPassCommands(CMD_EPASSISO7816)
{
}

EPassISO7816Commands::EPassISO7816Commands(std::string ct)
    : EPassCommands(ct)
{
}

EPassISO7816Commands::~EPassISO7816Commands() {}
}