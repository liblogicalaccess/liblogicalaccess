/**
 * \file generictagidondemandcommands.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Generic tag IdOnDemand commands.
 */

#ifndef LOGICALACCESS_GENERICTAGIDONDEMANDCOMMANDS_HPP
#define LOGICALACCESS_GENERICTAGIDONDEMANDCOMMANDS_HPP

#include <logicalaccess/cards/commands.hpp>
#include <logicalaccess/plugins/readers/idondemand/lla_readers_idondemand_api.hpp>

namespace logicalaccess
{
#define CMD_GENERICTAGIDONDEMAND "GenericTagIdOnDemand"

/**
 * \brief The Generic Tag IdOnDemand commands class.
 */
class LLA_READERS_IDONDEMAND_API GenericTagIdOnDemandCommands : public Commands
{
  public:
    GenericTagIdOnDemandCommands()
        : Commands(CMD_GENERICTAGIDONDEMAND)
    {
    }

    explicit GenericTagIdOnDemandCommands(std::string ct)
        : Commands(ct)
    {
    }
};
}

#endif