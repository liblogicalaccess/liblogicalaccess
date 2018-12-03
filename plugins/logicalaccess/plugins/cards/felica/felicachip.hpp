/**
 * \file felicachip.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief FeliCa chip.
 */

#ifndef LOGICALACCESS_FELICACHIP_HPP
#define LOGICALACCESS_FELICACHIP_HPP

#include <logicalaccess/cards/chip.hpp>
#include <logicalaccess/plugins/cards/felica/felicacommands.hpp>

#include <string>
#include <vector>
#include <iostream>

namespace logicalaccess
{
#define CHIP_FELICA "FeliCa"

#define FELICA_CODE_NDEF_READ 0x000B
#define FELICA_CODE_NDEF_WRITE 0x0009

/**
 * \brief The FeliCa base chip class.
 */
class LLA_CARDS_FELICA_API FeliCaChip : public Chip
{
  public:
    /**
     * \brief Constructor.
     */
    explicit FeliCaChip(std::string ct);

    /**
     * \brief Constructor.
     */
    FeliCaChip();

    /**
     * \brief Destructor.
     */
    virtual ~FeliCaChip();

    /**
     * \brief Get the generic card type.
     * \return The generic card type.
     */
    std::string getGenericCardType() const override
    {
        return CHIP_FELICA;
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
    * \brief Create default FeliCa location.
    * \return Default FeliCa location.
    */
    std::shared_ptr<Location> createLocation() const override;

    /**
     * \brief Get the FeliCa card provider for I/O access.
     * \return The FeliCa card provider.
     */
    std::shared_ptr<FeliCaCommands> getFeliCaCommands() const
    {
        return std::dynamic_pointer_cast<FeliCaCommands>(getCommands());
    }

  protected:
    static void addBlockNode(std::shared_ptr<LocationNode> rootNode, unsigned short code,
                             unsigned short block);
};
}

#endif