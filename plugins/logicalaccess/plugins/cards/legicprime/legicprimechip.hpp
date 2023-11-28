/**
 * \file legicprimechip.hpp
 * \author Maxime C. <maxime@leosac.com>
 * \brief Legic Prime chip.
 */

#ifndef LOGICALACCESS_LEGICPRIMECHIP_HPP
#define LOGICALACCESS_LEGICPRIMECHIP_HPP

#include <logicalaccess/cards/chip.hpp>
#include <logicalaccess/plugins/cards/legicprime/lla_cards_legicprime_api.hpp>
#include <string>
#include <vector>
#include <iostream>

namespace logicalaccess
{
#define CHIP_LEGICPRIME "LegicPrime"

/**
 * \brief The Legic Prime base chip class.
 */
class LLA_CARDS_LEGICPRIME_API LegicPrimeChip : public Chip
{
  public:
    /**
     * \brief Constructor.
     */
    LegicPrimeChip();

    /**
     * \brief Destructor.
     */
    virtual ~LegicPrimeChip();

    /**
     * \brief Get the generic card type.
     * \return The generic card type.
     */
    std::string getGenericCardType() const override
    {
        return CHIP_LEGICPRIME;
    }

    /**
     * \brief Get the root location node.
     * \return The root location node.
     */
    std::shared_ptr<LocationNode> getRootLocationNode() override;
};
}

#endif