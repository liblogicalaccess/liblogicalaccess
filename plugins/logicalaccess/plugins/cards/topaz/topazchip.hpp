/**
 * \file topazchip.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Topaz chip.
 */

#ifndef LOGICALACCESS_TOPAZCHIP_H
#define LOGICALACCESS_TOPAZCHIP_H

#include <logicalaccess/cards/chip.hpp>
#include <logicalaccess/plugins/cards/topaz/topazcommands.hpp>
#include <logicalaccess/plugins/cards/topaz/lla_cards_topaz_api.hpp>
#include <string>
#include <vector>
#include <iostream>

namespace logicalaccess
{
#define CHIP_TOPAZ "Topaz"

/**
 * \brief The Topaz base chip class.
 */
class LLA_CARDS_TOPAZ_API TopazChip : public Chip
{
  public:
    /**
     * \brief Constructor.
     */
    TopazChip();

    /**
     * \brief Destructor.
     */
    virtual ~TopazChip();

    /**
     * \brief Get the generic card type.
     * \return The generic card type.
     */
    std::string getGenericCardType() const override
    {
        return CHIP_TOPAZ;
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
     * \brief Get the number of blocks.
     * \return Number of blocks.
     */
    virtual unsigned short getNbBlocks(bool checkOnCard = false);

    /**
    * \brief Create default access informations.
    * \return Default access informations. Always null.
    */
    std::shared_ptr<AccessInfo> createAccessInfo() const override;

    /**
    * \brief Create default location.
    * \return Default location.
    */
    std::shared_ptr<Location> createLocation() const override;

    /**
     * \brief Get the Topaz commands.
     * \return The Topaz commands.
     */
    std::shared_ptr<TopazCommands> getTopazCommands() const
    {
        return std::dynamic_pointer_cast<TopazCommands>(getCommands());
    }

  protected:
    static void addBlockNode(std::shared_ptr<LocationNode> rootNode, int block);

    void checkRootLocationNodeName(std::shared_ptr<LocationNode> rootNode);

    unsigned short d_nbblocks;
};
}

#endif