/**
 * \file tagitcommands.hpp
 * \author Maxime C. <maxime@leosac.com>
 * \brief Tag-It commands.
 */

#ifndef LOGICALACCESS_TAGITCOMMANDS_HPP
#define LOGICALACCESS_TAGITCOMMANDS_HPP

#include <logicalaccess/key.hpp>
#include <logicalaccess/plugins/cards/iso15693/iso15693commands.hpp>
#include <logicalaccess/plugins/cards/tagit/lla_cards_tagit_api.hpp>

namespace logicalaccess
{
/**
 * \brief The Tag-It commands base class.
 */
class LLA_CARDS_TAGIT_API TagItCommands
{
  public:
    /**
     * \brief Get if a block is locked.
     * \param block The block number.
     * \return True if the block is locked, false otherwise.
     */
    static bool isLocked(std::shared_ptr<ISO15693Commands> cmd, unsigned char block);
};
}

#endif