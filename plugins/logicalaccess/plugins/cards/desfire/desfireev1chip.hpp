/**
 * \file desfireev1chip.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief DESFire EV1 chip.
 */

#ifndef LOGICALACCESS_DESFIREEV1CHIP_HPP
#define LOGICALACCESS_DESFIREEV1CHIP_HPP

#include <logicalaccess/plugins/cards/desfire/desfirechip.hpp>
#include <logicalaccess/plugins/cards/desfire/desfireev1commands.hpp>

namespace logicalaccess
{
#define CHIP_DESFIRE_EV1 "DESFireEV1"

/**
 * \brief The DESFire EV1 base chip class.
 */
class LIBLOGICALACCESS_API DESFireEV1Chip : public DESFireChip
{
  public:
    /**
     * \brief Constructor.
     */
    DESFireEV1Chip();

    /**
     * \brief Constructor.
     * \param ct The card type.
     */
    explicit DESFireEV1Chip(std::string ct);

    /**
     * \brief Destructor.
     */
    virtual ~DESFireEV1Chip();

    /**
     * \brief Get the root location node.
     * \return The root location node.
     */
    std::shared_ptr<LocationNode> getRootLocationNode() override;

    /**
     * \brief Get the application location information.
     * \return The location.
     */
    std::shared_ptr<DESFireLocation> getApplicationLocation() override;

    std::shared_ptr<CardService> getService(CardServiceType serviceType) override;

    /**
    * \brief Create default DESFire EV1 location.
    * \return Default DESFire EV1 location.
    */
    std::shared_ptr<Location> createLocation() const override;

    /**
     * \brief Get the DESFire card provider for I/O access.
     * \return The DESFire card provider.
     */
    std::shared_ptr<DESFireEV1Commands> getDESFireEV1Commands() const
    {
        return std::dynamic_pointer_cast<DESFireEV1Commands>(getCommands());
    }
};
}

#endif /* LOGICALACCESS_DESFIREEV1CHIP_HPP */