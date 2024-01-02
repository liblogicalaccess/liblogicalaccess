#pragma once

//#include <logicalaccess/plugins/cards/desfire/desfireev3commands.hpp>
#include <logicalaccess/plugins/cards/desfire/desfireev2chip.hpp>

namespace logicalaccess
{
#define CHIP_DESFIRE_EV3 "DESFireEV3"

/**
 * \brief The DESFire EV3 base chip class.
 */
class LLA_CARDS_DESFIRE_API DESFireEV3Chip : public DESFireEV2Chip
{
  public:
    /**
     * \brief Constructor.
     */
    DESFireEV3Chip();

    /**
     * \brief Destructor.
     */
    virtual ~DESFireEV3Chip() = default;

    /**
     * \brief Get the root location node.
     * \return The root location node.
     */
    std::shared_ptr<LocationNode> getRootLocationNode() override;

};
} // namespace logicalaccess
