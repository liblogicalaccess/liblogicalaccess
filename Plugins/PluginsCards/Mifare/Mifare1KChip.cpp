/**
 * \file Mifare1KChipe.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Mifare 1k chip.
 */

#include "Mifare1KChip.h"

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

