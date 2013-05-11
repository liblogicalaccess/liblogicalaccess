/**
 * \file Mifare1KChipe.cpp
 * \author Xavier SCHMERBER <xavier.schmerber@gmail.com>
 * \brief MifarePlus 2k chip.
 */

#include "MifarePlus2KChip.h"

namespace logicalaccess
{
	MifarePlus2KChip::MifarePlus2KChip()
		: MifarePlusChip("MifarePlus2K", 32)
	{
		d_nbSectors = 32;
	}

	MifarePlus2KChip::~MifarePlus2KChip()
	{		
	}
}