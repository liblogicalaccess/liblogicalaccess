/**
 * \file seoschip.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief SEOS chip.
 */

#ifndef LOGICALACCESS_SEOSCHIP_HPP
#define LOGICALACCESS_SEOSCHIP_HPP


#include <logicalaccess/plugins/cards/seos/lla_cards_seos_api.hpp>
#include <logicalaccess/plugins/cards/iso7816/iso7816chip.hpp>
#include <logicalaccess/plugins/cards/seos/seoscommands.hpp>

#include <string>
#include <vector>
#include <iostream>

namespace logicalaccess
{
#define CHIP_SEOS "Seos"

/**
 * \brief The HID SEOS base chip class.
 */
class LLA_CARDS_SEOS_API SeosChip : public Chip
{
  public:
    /**
     * \brief Constructor.
     */
    SeosChip();

    /**
     * \brief Destructor.
     */
    virtual ~SeosChip();

    /**
     * \brief Get the generic card type.
     * \return The generic card type.
     */
    std::string getGenericCardType() const override
    {
        return CHIP_SEOS;
    }

    /**
     * \brief Get the root location node.
     * \return The root location node.
     */
    std::shared_ptr<LocationNode> getRootLocationNode() override;

    std::shared_ptr<CardService> getService(CardServiceType serviceType) override;

	/**
     * \brief Get the SEOS commands for I/O access.
     * \return The SEOS commands.
     */
    std::shared_ptr<SeosCommands> getSeosCommands() const
    {
        return std::dynamic_pointer_cast<SeosCommands>(getCommands());
    }
};
}

#endif