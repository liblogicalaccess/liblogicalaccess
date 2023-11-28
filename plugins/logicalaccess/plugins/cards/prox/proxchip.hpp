/**
 * \file proxchip.hpp
 * \author Maxime C. <maxime@leosac.com>
 * \brief Prox chip.
 */

#ifndef LOGICALACCESS_PROXCHIP_HPP
#define LOGICALACCESS_PROXCHIP_HPP

#include <logicalaccess/cards/chip.hpp>
#include <logicalaccess/plugins/cards/prox/lla_cards_prox_api.hpp>
#include <string>
#include <vector>
#include <iostream>

namespace logicalaccess
{
#define CHIP_PROX "Prox"

/**
 * \brief The HID Prox base chip class.
 */
class LLA_CARDS_PROX_API ProxChip : public Chip
{
  public:
    /**
     * \brief Constructor.
     */
    ProxChip();

    /**
     * \brief Destructor.
     */
    virtual ~ProxChip();

    /**
     * \brief Get the generic card type.
     * \return The generic card type.
     */
    std::string getGenericCardType() const override
    {
        return CHIP_PROX;
    }

    /**
     * \brief Get the root location node.
     * \return The root location node.
     */
    std::shared_ptr<LocationNode> getRootLocationNode() override;

    /**
     * \brief Get a card service for this card provider.
     * \param serviceType The card service type.
     * \return The card service.
     */
    std::shared_ptr<CardService> getService(CardServiceType serviceType) override;

    /**
    * \brief Create default location.
    * \return Default location.
    */
    std::shared_ptr<Location> createLocation() const override;
};
}

#endif