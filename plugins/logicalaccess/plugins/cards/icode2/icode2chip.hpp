/**
 * \file icode2chip.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief iCode2 chip.
 */

#ifndef LOGICALACCESS_ICODE2CHIP_HPP
#define LOGICALACCESS_ICODE2CHIP_HPP

#include <logicalaccess/plugins/cards/iso15693/iso15693chip.hpp>
#include <logicalaccess/plugins/cards/icode2/lla_cards_icode2_api.hpp>
#include <string>
#include <vector>
#include <iostream>

namespace logicalaccess
{
#define CHIP_ICODE2 "iCode2"

/**
 * \brief The iCode2 base chip class.
 */
class LLA_CARDS_ICODE2_API ICode2Chip : public ISO15693Chip
{
  public:
    /**
     * \brief Constructor.
     */
    ICode2Chip();

    /**
     * \brief Destructor.
     */
    virtual ~ICode2Chip();

    /**
     * \brief Get the root location node.
     * \return The root location node.
     */
    std::shared_ptr<LocationNode> getRootLocationNode() override;
};
}

#endif