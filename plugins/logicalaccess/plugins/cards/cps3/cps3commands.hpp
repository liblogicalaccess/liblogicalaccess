/**
 * \file cps3commands.hpp
 * \author Maxime C. <maxime@leosac.com>
 * \brief CPS3 commands.
 */

#ifndef LOGICALACCESS_CPS3COMMANDS_HPP
#define LOGICALACCESS_CPS3COMMANDS_HPP

#include <logicalaccess/key.hpp>
#include <logicalaccess/plugins/cards/cps3/cps3location.hpp>
#include <logicalaccess/cards/commands.hpp>

namespace logicalaccess
{
#define CMD_CPS3 "CPS3"

/**
 * \brief The CPS3 commands class.
 */
class LLA_CARDS_CPS3_API CPS3Commands : public Commands
{
  public:
    CPS3Commands()
        : Commands(CMD_CPS3)
    {
    }

    explicit CPS3Commands(std::string ct)
        : Commands(ct)
    {
    }
};
}

#endif