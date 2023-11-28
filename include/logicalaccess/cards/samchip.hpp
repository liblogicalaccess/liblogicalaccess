/**
 * \file samchip.hpp
 * \author Maxime C. <maxime@leosac.com>
 * \brief SAM chip.
 */

#ifndef LOGICALACCESS_SAMCHIP_HPP
#define LOGICALACCESS_SAMCHIP_HPP

#include <logicalaccess/cards/chip.hpp>
#include <logicalaccess/lla_core_api.hpp>

namespace logicalaccess
{
#define CHIP_SAM "SAM"
/**
 * \brief The SAM chip base class.
 */
class LLA_CORE_API SAMChip : public Chip
{
  public:
    /**
     * \brief Constructor.
     */
    SAMChip()
        : Chip(CHIP_SAM)
    {
    }

    /**
     * \brief Constructor.
     */
    explicit SAMChip(std::string t)
        : Chip(t)
    {
    }

    /**
     * \brief Destructor.
     */
    ~SAMChip();

    /**
     * \brief Get the generic card type.
     * \return The generic card type.
     */
    std::string getGenericCardType() const override
    {
        return CHIP_SAM;
    }
};
}

#endif /* LOGICALACCESS_SAMCHIP_HPP */