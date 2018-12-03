/**
 * \file mifare4kchip.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Mifare 4k chip.
 */

#ifndef LOGICALACCESS_MIFARE4KCHIP_HPP
#define LOGICALACCESS_MIFARE4KCHIP_HPP

#include <logicalaccess/plugins/cards/mifare/mifarechip.hpp>

namespace logicalaccess
{
#define CHIP_MIFARE4K "Mifare4K"

/**
 * \brief The 4k Mifare base chip class.
 */
class LLA_CARDS_MIFARE_API Mifare4KChip : public MifareChip
{
  public:
    /**
     * \brief Constructor
     */
    Mifare4KChip();

    /**
     * \brief Destructor.
     */
    virtual ~Mifare4KChip();
};
}

#endif /* LOGICALACCESS_MIFARE4KCHIP_HPP */