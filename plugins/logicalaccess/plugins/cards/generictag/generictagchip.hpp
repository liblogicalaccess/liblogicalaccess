/**
 * \file generictagchip.hpp
 * \author Maxime C. <maxime@leosac.com>
 * \brief Generic tag chip.
 */

#ifndef LOGICALACCESS_GENERICTAGCHIP_HPP
#define LOGICALACCESS_GENERICTAGCHIP_HPP

#include <logicalaccess/cards/chip.hpp>
#include <logicalaccess/plugins/cards/generictag/lla_cards_generictag_api.hpp>
#include <string>
#include <vector>
#include <iostream>

namespace logicalaccess
{
/**
 * \brief The Generic Tag base chip class.
 */
class LLA_CARDS_GENERICTAG_API GenericTagChip : public Chip
{
  public:
    /**
     * \brief Constructor.
     * \param cardtype The card type.
     */
    explicit GenericTagChip(std::string cardtype);

    /**
     * \brief Constructor.
     */
    GenericTagChip();

    /**
     * \brief Destructor.
     */
    virtual ~GenericTagChip();

    /**
     * \brief Get the generic card type.
     * \return The generic card type.
     */
    std::string getGenericCardType() const override
    {
        return CHIP_GENERICTAG;
    }

    /**
     * \brief Get the root location node.
     * \return The root location node.
     */
    std::shared_ptr<LocationNode> getRootLocationNode() override;

    void setTagIdBitsLength(unsigned int length)
    {
        d_tagIdBitsLength = length;
    }

    unsigned int getTagIdBitsLength() const
    {
        return d_tagIdBitsLength;
    }

    /**
     * \brief Get a card service for this card provider.
     * \param serviceType The card service type.
     * \return The card service.
     */
    std::shared_ptr<CardService> getService(CardServiceType serviceType) override;

    void setRealChip(std::shared_ptr<Chip> real_chip);

  protected:
    /**
     * \brief The tag id length in bits (if 0, the length is the full bytes).
     */
    unsigned int d_tagIdBitsLength;

    std::shared_ptr<Chip> d_real_chip;
};
}

#endif