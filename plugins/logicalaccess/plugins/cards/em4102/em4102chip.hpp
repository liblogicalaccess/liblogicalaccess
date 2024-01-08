/**
 * \file em4102chip.hpp
 * \author Maxime C. <maxime@leosac.com>
 * \brief EM4102 chip.
 */

#ifndef LOGICALACCESS_EM4102CHIP_HPP
#define LOGICALACCESS_EM4102CHIP_HPP

#include <logicalaccess/cards/chip.hpp>
#include <logicalaccess/plugins/cards/em4102/lla_cards_em4102_api.hpp>
#include <string>
#include <vector>
#include <iostream>

namespace logicalaccess
{
#define CHIP_EM4102 "EM4102"
/**
 * \brief The EM4102 base chip class.
 */
class LLA_CARDS_EM4102_API EM4102Chip : public Chip
{
  public:
    /**
     * \brief Constructor.
     */
    EM4102Chip();

    /**
     * \brief Destructor.
     */
    virtual ~EM4102Chip();

    /**
     * \brief Get the generic card type.
     * \return The generic card type.
     */
    std::string getGenericCardType() const override
    {
        return CHIP_EM4102;
    }

    /**
     * \brief Get the root location node.
     * \return The root location node.
     */
    std::shared_ptr<LocationNode> getRootLocationNode() override;
};
}

#endif