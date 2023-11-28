/**
 * \file yubikeychip.hpp
 * \author Maxime C. <maxime@leosac.com>
 * \brief Yubikey chip.
 */

#ifndef LOGICALACCESS_YUBIKEYCHIP_HPP
#define LOGICALACCESS_YUBIKEYCHIP_HPP

#include <logicalaccess/cards/chip.hpp>
#include <logicalaccess/plugins/cards/yubikey/yubikeycommands.hpp>
#include <logicalaccess/plugins/cards/yubikey/lla_cards_yubikey_api.hpp>

#include <string>
#include <vector>
#include <iostream>

namespace logicalaccess
{
#define CHIP_YUBIKEY "Yubikey"

/**
 * \brief The Yubikey base chip class.
 */
class LLA_CARDS_YUBIKEY_API YubikeyChip : public Chip
{
  public:
    /**
     * \brief Constructor.
     */
    explicit YubikeyChip(std::string ct);

    /**
     * \brief Constructor.
     */
    YubikeyChip();

    /**
     * \brief Destructor.
     */
    virtual ~YubikeyChip();

    /**
     * \brief Get the generic card type.
     * \return The generic card type.
     */
    std::string getGenericCardType() const override
    {
        return CHIP_YUBIKEY;
    }

    /**
     * \brief Get a card service for this chip.
     * \param serviceType The card service type.
     * \return The card service.
     */
    std::shared_ptr<CardService> getService(CardServiceType serviceType) override;
	
	/**
     * \brief Get the Yubikey commands.
     * \return The Yubikey commands.
     */
    std::shared_ptr<YubikeyCommands> getYubikeyCommands() const
    {
        return std::dynamic_pointer_cast<YubikeyCommands>(getCommands());
    }
};
}

#endif