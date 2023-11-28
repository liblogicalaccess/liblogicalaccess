/**
 * \file iso15693chip.hpp
 * \author Maxime C. <maxime@leosac.com>
 * \brief ISO15693 chip.
 */

#ifndef LOGICALACCESS_ISO15693CHIP_HPP
#define LOGICALACCESS_ISO15693CHIP_HPP

#include <logicalaccess/cards/chip.hpp>
#include <logicalaccess/plugins/cards/iso15693/iso15693commands.hpp>
#include <logicalaccess/plugins/cards/iso15693/lla_cards_iso15693_api.hpp>
#include <string>
#include <vector>
#include <iostream>

namespace logicalaccess
{
#define CHIP_ISO15693 "ISO15693"

/**
 * \brief The ISO15693 base chip class.
 */
class LLA_CARDS_ISO15693_API ISO15693Chip : public Chip
{
  public:
    /**
     * \brief Constructor.
     */
    explicit ISO15693Chip(std::string ct);

    /**
     * \brief Constructor.
     */
    ISO15693Chip();

    /**
     * \brief Destructor.
     */
    virtual ~ISO15693Chip();

    /**
     * \brief Get the generic card type.
     * \return The generic card type.
     */
    std::string getGenericCardType() const override
    {
        return CHIP_ISO15693;
    }

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
    * \brief Create default ISO15693 location.
    * \return Default ISO15693 location.
    */
    std::shared_ptr<Location> createLocation() const override;

    /**
     * \brief Get the ISO15693 card provider for I/O access.
     * \return The ISO15693 card provider.
     */
    std::shared_ptr<ISO15693Commands> getISO15693Commands() const
    {
        return std::dynamic_pointer_cast<ISO15693Commands>(getCommands());
    }
};
}

#endif