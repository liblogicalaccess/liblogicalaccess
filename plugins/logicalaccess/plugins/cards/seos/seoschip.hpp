/**
 * \file seoschip.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief SEOS chip.
 */

#ifndef LOGICALACCESS_SEOSCHIP_HPP
#define LOGICALACCESS_SEOSCHIP_HPP

#include <logicalaccess/cards/chip.hpp>

#include <string>
#include <vector>
#include <iostream>

namespace logicalaccess
{
#define CHIP_SEOS "SEOS"

/**
 * \brief The HID SEOS base chip class.
 */
class LIBLOGICALACCESS_API SEOSChip : public Chip
{
  public:
    /**
     * \brief Constructor.
     */
    SEOSChip();

    /**
     * \brief Destructor.
     */
    virtual ~SEOSChip();

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
};
}

#endif