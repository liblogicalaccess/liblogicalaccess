/**
 * \file desfireev2chip.hpp
 * \author Maxime C. <maxime@leosac.com>
 * \brief DESFire EV2 chip.
 */

#ifndef LOGICALACCESS_DESFIREEV2CHIP_HPP
#define LOGICALACCESS_DESFIREEV2CHIP_HPP

#include <logicalaccess/plugins/cards/desfire/desfireev1chip.hpp>
#include <logicalaccess/plugins/cards/desfire/desfireev2commands.hpp>

namespace logicalaccess
{
#define CHIP_DESFIRE_EV2 "DESFireEV2"

/**
 * \brief The DESFire EV2 base chip class.
 */
class LLA_CARDS_DESFIRE_API DESFireEV2Chip : public DESFireEV1Chip
{
  public:
    /**
     * \brief Constructor.
     */
    DESFireEV2Chip();

    explicit DESFireEV2Chip(const std::string &ct);

    /**
     * \brief Destructor.
     */
    virtual ~DESFireEV2Chip();

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
     * \brief Create default DESFire EV2 location.
     * \return Default DESFire EV2 location.
     */
    std::shared_ptr<Location> createLocation() const override;

    /**
     * \brief Get the DESFire card provider for I/O access.
     * \return The DESFire card provider.
     */
    std::shared_ptr<DESFireEV2Commands> getDESFireEV2Commands() const
    {
        return std::dynamic_pointer_cast<DESFireEV2Commands>(getCommands());
    }
};
} // namespace logicalaccess

#endif /* LOGICALACCESS_DESFIREEV2CHIP_HPP */