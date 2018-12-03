/**
 * \file mifarechip.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Mifare chip.
 */

#ifndef LOGICALACCESS_MIFARECHIP_HPP
#define LOGICALACCESS_MIFARECHIP_HPP

#include <logicalaccess/cards/chip.hpp>
#include <logicalaccess/plugins/cards/mifare/mifarecommands.hpp>
#include <logicalaccess/plugins/cards/mifare/lla_cards_mifare_api.hpp>
#include <string>
#include <vector>
#include <iostream>

namespace logicalaccess
{
#define CHIP_MIFARE "Mifare"

/**
 * \brief The Mifare base chip class.
 */
class LLA_CARDS_MIFARE_API MifareChip : public Chip
{
  public:
    /**
     * \brief Constructor for generic Mifare.
     */
    MifareChip();

    /**
    * \brief Constructor for generic Mifare.
    */
    explicit MifareChip(std::string cardtype);

    /**
     * \brief Destructor.
     */
    virtual ~MifareChip();

    /**
     * \brief Get the generic card type.
     * \return The generic card type.
     */
    std::string getGenericCardType() const override
    {
        return CHIP_MIFARE;
    }

    /**
     * \brief Get the root location node.
     * \return The root location node.
     */
    std::shared_ptr<LocationNode> getRootLocationNode() override;

    /**
     * \brief Get number of sectors
     */
    unsigned int getNbSectors() const;

    /**
     * \brief Add a sector node to a root node.
     * \param rootNode The root node.
     * \param sector The sector.
     */
    static void addSectorNode(std::shared_ptr<LocationNode> rootNode, int sector);

    /**
    * \brief Add a block node to a root node.
    * \param rootNode The root node.
    * \param sector The sector.
    * \param block The block.
    */
    static void addBlockNode(std::shared_ptr<LocationNode> rootNode, int sector,
                             unsigned char block);

    /**
     * \brief Get a card service for this card provider.
     * \param serviceType The card service type.
     * \return The card service.
     */
    std::shared_ptr<CardService> getService(CardServiceType serviceType) override;

    /**
    * \brief Create default Mifare access informations.
    * \return Default Mifare access informations.
    */
    std::shared_ptr<AccessInfo> createAccessInfo() const override;

    /**
    * \brief Create default Mifare location.
    * \return Default Mifare location.
    */
    std::shared_ptr<Location> createLocation() const override;

    /**
     * \brief Get the Mifare commands.
     * \return The Mifare commands.
     */
    std::shared_ptr<MifareCommands> getMifareCommands() const
    {
        return std::dynamic_pointer_cast<MifareCommands>(getCommands());
    }

  protected:
    /**
     * \brief Constructor.
     * \param cardtype The card type (Mifare 1K, Mifare 4K, ...).
     * \param nbSectors The number of sectors in the card.
     */
    MifareChip(std::string cardtype, unsigned int nbSectors);

    /**
     * \brief The number of sectors in the Mifare card.
     */
    unsigned int d_nbSectors;
};
}

#endif /* LOGICALACCESS_MIFARECHIP_HPP */