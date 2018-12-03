/**
 * \file infineonmydchip.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Infineon My-D chip.
 */

#ifndef LOGICALACCESS_INFINEONMYDCHIP_HPP
#define LOGICALACCESS_INFINEONMYDCHIP_HPP

#include <logicalaccess/plugins/cards/iso15693/iso15693chip.hpp>
#include <logicalaccess/plugins/cards/infineonmyd/lla_cards_infineonmyd_api.hpp>
#include <string>
#include <vector>
#include <iostream>

namespace logicalaccess
{
#define CHIP_INFINEONMYD "InfineonMYD"

/**
 * \brief The Infineon My-D base chip class.
 */
class LLA_CARDS_INFINEONMYD_API InfineonMYDChip : public ISO15693Chip
{
  public:
    /**
     * \brief Constructor.
     */
    InfineonMYDChip();

    /**
     * \brief Destructor.
     */
    virtual ~InfineonMYDChip();

    /**
     * \brief Get the root location node.
     * \return The root location node.
     */
    std::shared_ptr<LocationNode> getRootLocationNode() override;
};
}

#endif