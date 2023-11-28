/**
 * \file icode1chip.hpp
 * \author Maxime C. <maxime@leosac.com>
 * \brief iCode1 chip.
 */

#ifndef LOGICALACCESS_ICODE1CHIP_HPP
#define LOGICALACCESS_ICODE1CHIP_HPP

#include <logicalaccess/plugins/cards/iso15693/iso15693chip.hpp>
#include <logicalaccess/plugins/cards/icode1/lla_cards_icode1_api.hpp>
#include <string>
#include <vector>
#include <iostream>

namespace logicalaccess
{
#define CHIP_ICODE1 "iCode1"

/**
 * \brief The iCode1 base chip class.
 */
class LLA_CARDS_ICODE1_API ICode1Chip : public ISO15693Chip
{
  public:
    /**
     * \brief Constructor.
     */
    ICode1Chip();

    /**
     * \brief Destructor.
     */
    virtual ~ICode1Chip();

    /**
     * \brief Get the root location node.
     * \return The root location node.
     */
    std::shared_ptr<LocationNode> getRootLocationNode() override;
};
}

#endif