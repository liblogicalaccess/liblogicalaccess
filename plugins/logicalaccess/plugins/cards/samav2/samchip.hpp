/**
 * \file desfireev1chip.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief DESFire EV1 chip.
 */

#ifndef LOGICALACCESS_SAMCHIP_HPP
#define LOGICALACCESS_SAMCHIP_HPP

#include <logicalaccess/cards/chip.hpp>
#include <logicalaccess/plugins/cards/samav2/samcommands.hpp>

namespace logicalaccess
{
#define CHIP_SAM "SAM"
/**
 * \brief The SAM chip class.
 */
class LIBLOGICALACCESS_API SAMChip : public Chip
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
    ~SAMChip()
    {
    }

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

#endif /* LOGICALACCESS_DESFIREEV1CHIP_HPP */