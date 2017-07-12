/**
 * \file generictagidondemandcommands.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Generic tag IdOnDemand commands.
 */

#ifndef LOGICALACCESS_GENERICTAGIDONDEMANDCOMMANDS_HPP
#define LOGICALACCESS_GENERICTAGIDONDEMANDCOMMANDS_HPP

#include "logicalaccess/cards/commands.hpp"

namespace logicalaccess
{
#define CMD_GENERICTAGIDONDEMAND "GenericTagIdOnDemand"

    /**
     * \brief The Generic Tag IdOnDemand commands class.
     */
    class LIBLOGICALACCESS_API GenericTagIdOnDemandCommands : public Commands
    {
    public:

		GenericTagIdOnDemandCommands() : Commands(CMD_GENERICTAGIDONDEMAND) {}

		GenericTagIdOnDemandCommands(std::string ct) : Commands(ct) {}
    };
}

#endif