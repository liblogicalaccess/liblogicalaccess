/**
 * \file Mifare4KChip.cpp
 * \author Maxime CHAMLEY <maxime.chamley@islog.eu>
 * \brief Mifare 4k chip.
 */

#include "Mifare4KChip.h"

namespace LOGICALACCESS
{
	Mifare4KChip::Mifare4KChip()
		: MifareChip("Mifare4K", 40)
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

