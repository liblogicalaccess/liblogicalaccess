/**
 * \file twicchip.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Twic chip.
 */

#ifndef LOGICALACCESS_TWICCHIP_HPP
#define LOGICALACCESS_TWICCHIP_HPP

#include <logicalaccess/plugins/cards/iso7816/iso7816chip.hpp>
#include <logicalaccess/plugins/cards/twic/twiccommands.hpp>

#include <string>
#include <vector>
#include <iostream>

namespace logicalaccess
{
#define CHIP_TWIC "Twic"

/**
 * \brief The Twic base chip class.
 */
class LIBLOGICALACCESS_API TwicChip : public ISO7816Chip
{
  public:
    /**
     * \brief Constructor.
     */
    TwicChip();

    /**
     * \brief Destructor.
     */
    virtual ~TwicChip();

    /**
     * \brief Get the root location node.
     * \return The root location node.
     */
    std::shared_ptr<LocationNode> getRootLocationNode() override;

    /**
     * \brief Get a card service for this chip.
     * \param serviceType The card service type.
     * \return The card service.
     */
    std::shared_ptr<CardService> getService(CardServiceType serviceType) override;

    /**
    * \brief Create default Twic location.
    * \return Default Twic location.
    */
    std::shared_ptr<Location> createLocation() const override;

    /**
     * \brief Get the Twic commands.
     * \return The Twic commands.
     */
    std::shared_ptr<TwicCommands> getTwicCommands() const
    {
        return std::dynamic_pointer_cast<TwicCommands>(getISO7816Commands());
    }
};
}

#endif