/**
 * \file smartframechip.hpp
 * \author Maxime C. <maxime@leosac.com>
 * \brief Smart Frame chip.
 */

#ifndef LOGICALACCESS_SMARTFRAMECHIP_HPP
#define LOGICALACCESS_SMARTFRAMECHIP_HPP

#include <logicalaccess/cards/chip.hpp>
#include <logicalaccess/plugins/cards/smartframe/lla_cards_smartframe_api.hpp>
#include <string>
#include <vector>
#include <iostream>

namespace logicalaccess
{
#define CHIP_SMARTFRAME "SmartFrame"

/**
 * \brief The Smart Frame base chip class.
 */
class LLA_CARDS_SMARTFRAME_API SmartFrameChip : public Chip
{
  public:
    /**
     * \brief Constructor.
     */
    SmartFrameChip();

    /**
     * \brief Destructor.
     */
    virtual ~SmartFrameChip();

    /**
     * \brief Get the generic card type.
     * \return The generic card type.
     */
    std::string getGenericCardType() const override
    {
        return CHIP_SMARTFRAME;
    }

    /**
     * \brief Get the root location node.
     * \return The root location node.
     */
    std::shared_ptr<LocationNode> getRootLocationNode() override;
};
}

#endif