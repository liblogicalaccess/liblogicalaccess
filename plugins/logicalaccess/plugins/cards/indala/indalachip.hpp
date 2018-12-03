/**
 * \file indalachip.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Indala chip.
 */

#ifndef LOGICALACCESS_INDALACHIP_HPP
#define LOGICALACCESS_INDALACHIP_HPP

#include <logicalaccess/cards/chip.hpp>
#include <logicalaccess/plugins/cards/indala/lla_cards_indala_api.hpp>
#include <string>
#include <vector>
#include <iostream>

namespace logicalaccess
{
#define CHIP_INDALA "Indala"

/**
 * \brief The Indala base chip class.
 */
class LLA_CARDS_INDALA_API IndalaChip : public Chip
{
  public:
    /**
     * \brief Constructor.
     */
    IndalaChip();

    /**
     * \brief Destructor.
     */
    virtual ~IndalaChip();

    /**
     * \brief Get the generic card type.
     * \return The generic card type.
     */
    std::string getGenericCardType() const override
    {
        return CHIP_INDALA;
    }

    /**
     * \brief Get the root location node.
     * \return The root location node.
     */
    std::shared_ptr<LocationNode> getRootLocationNode() override;
};
}

#endif