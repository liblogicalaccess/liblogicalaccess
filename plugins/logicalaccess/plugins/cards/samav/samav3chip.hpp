/**
 * \file samav3chip.hpp
 * \author Maxime C. <maxime@leosac.com>
 * \brief SAMAV3Chip header.
 */

#ifndef LOGICALACCESS_SAMAV3CHIP_HPP
#define LOGICALACCESS_SAMAV3CHIP_HPP

#include <logicalaccess/cards/samchip.hpp>
#include <logicalaccess/plugins/cards/samav/samcommands.hpp>

namespace logicalaccess
{
#define CHIP_SAMAV3 "SAM_AV3"

/**
 * \brief The SAM chip class.
 */
class LLA_CARDS_SAMAV_API SAMAV3Chip : public SAMChip
{
  public:
    /**
     * \brief Constructor.
     */
    SAMAV3Chip();

    explicit SAMAV3Chip(std::string ct);

    /**
     * \brief Destructor.
     */
    ~SAMAV3Chip();

    std::shared_ptr<SAMCommands<KeyEntryAV2Information, SETAV2>> getSAMAV3Commands() const
    {
        return std::dynamic_pointer_cast<SAMCommands<KeyEntryAV2Information, SETAV2>>(
            getCommands());
    }
};
}

#endif /* LOGICALACCESS_SAMAV3CHIP_HPP */