/**
 * \file mifare4kchip.cpp
 * \author Maxime C. <maxime@leosac.com>
 * \brief Mifare 4k chip.
 */

#include <logicalaccess/plugins/cards/mifare/mifare4kchip.hpp>

namespace logicalaccess
{
Mifare4KChip::Mifare4KChip()
    : MifareChip(CHIP_MIFARE4K, 40)
{
}

Mifare4KChip::~Mifare4KChip()
{
}
}