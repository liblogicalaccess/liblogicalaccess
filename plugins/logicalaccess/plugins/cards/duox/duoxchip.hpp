#pragma once

#include <logicalaccess/plugins/cards/duox/duoxcommands.hpp>
#include <logicalaccess/plugins/cards/desfire/desfireev3chip.hpp>

namespace logicalaccess
{

#define CHIP_DUOX "DUOX"

/**
 * \brief Base chip implementation for DUOX cards.
 * 
 * DUOX is based on the DESFire EV3 architecture and therefore inherits the DESFire EV3 chip behavior by default.
 * 
 * This class intentionally remains non-final so that DUOX-specific variants can derive from it.
 */
class LLA_CARDS_DESFIRE_API DUOXChip : public DESFireEV3Chip
{
public:
    /**
     * \brief Constructor.
     */
    DUOXChip();

    /**
     * \brief Destructor.
     */
    ~DUOXChip() override = default;

    /**
     * \brief Get the DUOX card commands.
     * \return The DUOX card commands.
     */
    std::shared_ptr<DUOXCommands> getDUOXCommands() const
    {
        return std::dynamic_pointer_cast<DUOXCommands>(getCommands());
    }

    /**
     * \brief Get the root location node.
     * \return The root location node.
     */
    std::shared_ptr<LocationNode> getRootLocationNode() override;
};

} // namespace logicalaccess