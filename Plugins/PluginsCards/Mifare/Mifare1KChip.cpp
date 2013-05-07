/**
 * \file Mifare1KChipe.cpp
 * \author Maxime CHAMLEY <maxime.chamley@islog.eu>
 * \brief Mifare 1k chip.
 */

#include "Mifare1KChip.h"

namespace LOGICALACCESS
{
	Mifare1KChip::Mifare1KChip()
		: MifareChip(CHIP_MIFARE1K, 16)
	{
		
	}

	Mifare1KChip::~Mifare1KChip()
	{		
	}
}

