#pragma once

#include <logicalaccess/plugins/cards/desfire/desfireev3commands.hpp>
#include <logicalaccess/plugins/cards/desfire/desfireev2chip.hpp>

namespace logicalaccess
{
#define CHIP_DESFIRE_EV3 "DESFireEV3"

/**
 * \brief Base chip class for MIFARE DESFire EV3 cards.
 */
class LLA_CARDS_DESFIRE_API DESFireEV3Chip : public DESFireEV2Chip
{
  public:
    /**
     * \brief Default constructor.
     */
    DESFireEV3Chip();

    /**
     * \brief Constructor using a custom chip type.
     *
     * \param ct Chip type identifier.
     */
    explicit DESFireEV3Chip(const std::string &ct);

    /**
     * \brief Destructor.
     */
    virtual ~DESFireEV3Chip() = default;

    /**
     * \brief Get the DESFire EV3 card commands.
     * \return The DESFire EV3 card commands associated with this chip.
     */
    std::shared_ptr<DESFireEV3Commands> getDESFireEV3Commands() const
    {
        return std::dynamic_pointer_cast<DESFireEV3Commands>(getCommands());
    }

    /**
     * \brief Get the root location node.
     * \return The root location node.
     */
    std::shared_ptr<LocationNode> getRootLocationNode() override;

    /**
     * \brief Get a card service.
     *
     * \param serviceType Type of the requested card service.
     *
     * \return The requested card service.
     */
    std::shared_ptr<CardService> getService(CardServiceType serviceType) override;

};
} // namespace logicalaccess
