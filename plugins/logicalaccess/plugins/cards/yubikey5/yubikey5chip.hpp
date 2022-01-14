/**
 * \file yubikey5chip.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Yubikey5 chip.
 */

#ifndef LOGICALACCESS_YUBIKEY5CHIP_HPP
#define LOGICALACCESS_YUBIKEY5CHIP_HPP

#include <logicalaccess/plugins/cards/iso7816/iso7816chip.hpp>
#include <logicalaccess/plugins/cards/yubikey5/lla_cards_yubikey5_api.hpp>

#include <string>
#include <vector>
#include <iostream>

namespace logicalaccess
{
#define CHIP_YUBIKEY5 "Yubikey5"

/**
 * \brief The Yubikey5 base chip class.
 */
class LLA_CARDS_YUBIKEY5_API Yubikey5Chip : public ISO7816Chip
{
  public:
    /**
     * \brief Constructor.
     */
    explicit Yubikey5Chip(std::string ct);

    /**
     * \brief Constructor.
     */
    Yubikey5Chip();

    /**
     * \brief Destructor.
     */
    virtual ~Yubikey5Chip();

    /**
     * \brief Get the generic card type.
     * \return The generic card type.
     */
    std::string getGenericCardType() const override
    {
        return CHIP_YUBIKEY5;
    }

    /**
     * \brief Get a card service for this chip.
     * \param serviceType The card service type.
     * \return The card service.
     */
    //std::shared_ptr<CardService> getService(CardServiceType serviceType) override;
};
}

#endif