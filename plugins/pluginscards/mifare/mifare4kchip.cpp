/**
 * \file mifare4kchip.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Mifare 4k chip.
 */

#include "mifare4kchip.hpp"

namespace logicalaccess
{
    Mifare4KChip::Mifare4KChip()
        : MifareChip(CHIP_MIFARE4K, 40)
    {
    }

    Mifare4KChip::~Mifare4KChip()
    {
    }

    boost::shared_ptr<LocationNode> Mifare4KChip::getRootLocationNode()
    {
        boost::shared_ptr<LocationNode> rootNode = MifareChip::getRootLocationNode();

        for (int i = 16; i < 40; i++)
        {
            addSectorNode(rootNode, i);
        }

        return rootNode;
    }
}