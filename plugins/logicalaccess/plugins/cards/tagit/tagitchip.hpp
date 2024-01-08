/**
 * \file tagitchip.hpp
 * \author Maxime C. <maxime@leosac.com>
 * \brief Tag-It chip.
 */

#ifndef LOGICALACCESS_TAGITCHIP_HPP
#define LOGICALACCESS_TAGITCHIP_HPP

#include <logicalaccess/plugins/cards/iso15693/iso15693chip.hpp>
#include <logicalaccess/plugins/cards/tagit/tagitcommands.hpp>
#include <logicalaccess/plugins/cards/tagit/lla_cards_tagit_api.hpp>
#include <string>
#include <vector>
#include <iostream>

namespace logicalaccess
{
#define CHIP_TAGIT "TagIt"

/**
 * \brief The Tag-It base chip class.
 */
class LLA_CARDS_TAGIT_API TagItChip : public ISO15693Chip
{
  public:
    /**
     * \brief Constructor.
     */
    TagItChip();

    /**
     * \brief Destructor.
     */
    virtual ~TagItChip();

    /**
     * \brief Get the root location node.
     * \return The root location node.
     */
    std::shared_ptr<LocationNode> getRootLocationNode() override;
};
}

#endif