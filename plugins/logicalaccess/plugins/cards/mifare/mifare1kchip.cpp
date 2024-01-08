/**
 * \file mifare1kchipe.cpp
 * \author Maxime C. <maxime@leosac.com>
 * \brief Mifare 1k chip.
 */

#include <logicalaccess/plugins/cards/mifare/mifare1kchip.hpp>

namespace logicalaccess
{
Mifare1KChip::Mifare1KChip()
    : MifareChip(CHIP_MIFARE1K, 16)
{
}

Mifare1KChip::~Mifare1KChip()
{
}
}