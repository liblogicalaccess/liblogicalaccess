/**
 * \file mifare1kchipe.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Mifare 1k chip.
 */

#include "mifare1kchip.hpp"

namespace logicalaccess
{
    Mifare1KChip::Mifare1KChip()
		: Chip(CHIP_MIFARE1K),
		MifareChip(CHIP_MIFARE1K, 16)
    {
    }

    Mifare1KChip::~Mifare1KChip()
    {
    }
}