/**
 * \file mifareplus4kchip.cpp
 * \author Xavier SCHMERBER <xavier.schmerber@gmail.com
 * \brief MifarePlus 4k chip.
 */

#include "mifareplus4kchip.hpp"

namespace logicalaccess
{
    MifarePlus4KChip::MifarePlus4KChip()
        : MifarePlusChip(CHIP_MIFAREPLUS4K, 40)
    {
        d_nbSectors = 40;
    }

    MifarePlus4KChip::~MifarePlus4KChip()
    {
    }

    std::shared_ptr<LocationNode> MifarePlus4KChip::getRootLocationNode()
    {
        std::shared_ptr<LocationNode> rootNode = MifarePlusChip::getRootLocationNode();

        for (int i = 32; i < 40; i++)
        {
            addSectorNode(rootNode, i);
        }

        return rootNode;
    }
}