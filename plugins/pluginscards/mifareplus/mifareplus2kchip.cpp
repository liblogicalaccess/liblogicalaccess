/**
 * \file mifare1kchipe.cpp
 * \author Xavier SCHMERBER <xavier.schmerber@gmail.com>
 * \brief MifarePlus 2k chip.
 */

#include "mifareplus2kchip.hpp"

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