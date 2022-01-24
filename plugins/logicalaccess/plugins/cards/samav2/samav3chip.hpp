#pragma once

#include <logicalaccess/cards/samchip.hpp>
#include <logicalaccess/plugins/cards/samav2/samav3commands.hpp>

namespace logicalaccess
{
#define CHIP_SAMAV3 "SAMAV3"

/**
 *
 */
class LLA_CARDS_SAMAV2_API SAMAV3Chip : public SAMChip
{
  public:
    /**
     * \brief Constructor.
     */
    SAMAV3Chip();

    /**
     * \brief Destructor.
     */
    ~SAMAV3Chip() = default;

    std::shared_ptr<SAMAV3Commands> getSAMAV3Commands() const
    {
        return std::dynamic_pointer_cast<SAMAV3Commands>(getCommands());
    }
};
}