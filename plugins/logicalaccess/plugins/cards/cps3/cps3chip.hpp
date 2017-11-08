/**
 * \file cps3chip.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief CPS3 chip.
 */

#ifndef LOGICALACCESS_CPS3CHIP_HPP
#define LOGICALACCESS_CPS3CHIP_HPP

#include <logicalaccess/plugins/cards/iso7816/iso7816chip.hpp>
#include <logicalaccess/plugins/cards/cps3/cps3commands.hpp>

#include <string>
#include <vector>
#include <iostream>

namespace logicalaccess
{
#define CHIP_CPS3 "CPS3"

/**
 * \brief The CPS3 base chip class.
 */
class LIBLOGICALACCESS_API CPS3Chip : public ISO7816Chip
{
  public:
    /**
     * \brief Constructor.
     */
    explicit CPS3Chip(std::string ct);

    /**
     * \brief Constructor.
     */
    CPS3Chip();

    /**
     * \brief Destructor.
     */
    virtual ~CPS3Chip();

    /**
     * \brief Get the generic card type.
     * \return The generic card type.
     */
    std::string getGenericCardType() const override
    {
        return CHIP_CPS3;
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
    * \brief Create default CPS3 location.
    * \return Default CPS3 location.
    */
    std::shared_ptr<Location> createLocation() const override;

    /**
     * \brief Get the CPS3 card provider for I/O access.
     * \return The CPS3 card provider.
     */
    std::shared_ptr<CPS3Commands> getCPS3Commands() const
    {
        return std::dynamic_pointer_cast<CPS3Commands>(getCommands());
    }
};
}

#endif