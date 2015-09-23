/**
 * \file mifarechip.cpp
 * \author Xavier SCHMERBER <xavier.schmerber@gmail.com>
 * \brief Mifare chip.
 */

#include "mifarepluschip.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>

#include "logicalaccess/services/accesscontrol/accesscontrolcardservice.hpp"
#include "logicalaccess/services/accesscontrol/formats/bithelper.hpp"
#include "logicalaccess/cards/locationnode.hpp"
#include "../mifare/mifarestoragecardservice.hpp"
#include <cassert>

#define MIFARE_PLUS_2K_SECTOR_NB 32

namespace logicalaccess
{
    uint16_t MifarePlusChip::key_number_from_sector(int sector, MifareKeyType type)
    {
        int offset = sector * 2;
        if (type == MifareKeyType::KT_KEY_B)
            offset++;

        uint16_t pos = 0;
        pos |= 0x40 << 8;
        pos |= offset & 0xFF;

        LOG(DEBUGS) << "Pos = " << std::hex << pos;

        assert(pos >= 0x4000);
        assert(pos <= 0x403F);
        return pos;
    }


	std::string MifarePlusChip::getGenericCardType() const
	{
		return "MifarePlus";
	}
}