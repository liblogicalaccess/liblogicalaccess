/**
 * \file mifarechip.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Mifare chip.
 */

#include "mifarechip.hpp"
#include "mifareprofile.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>

#include "logicalaccess/services/accesscontrol/formats/bithelper.hpp"


namespace logicalaccess
{	
	MifareChip::MifareChip()
		: Chip("Mifare"), d_nbSectors(16)
	{
		d_profile.reset(new MifareProfile());
	}

	MifareChip::MifareChip(std::string cardtype, unsigned int nbSectors) : 
		Chip(cardtype), d_nbSectors(nbSectors)
	{
		d_profile.reset(new MifareProfile());
	}

	MifareChip::~MifareChip()
	{
		
	}	

	unsigned int MifareChip::getNbSectors() const
	{
		return d_nbSectors;
	}

	void MifareChip::addSectorNode(boost::shared_ptr<LocationNode> rootNode, int sector)
	{
		char tmpName[255];
		boost::shared_ptr<LocationNode> sectorNode;
		sectorNode.reset(new LocationNode());

		sprintf(tmpName, "Sector %u", sector);
		sectorNode->setName(tmpName);
		sectorNode->setLength((sector >= 32) ? 240 : 48);
		sectorNode->setNeedAuthentication(true);

		boost::shared_ptr<MifareLocation> location;
		location.reset(new MifareLocation());
		location->sector = sector;
		location->byte = 0;

		sectorNode->setLocation(location);
		sectorNode->setParent(rootNode);
		rootNode->getChildrens().push_back(sectorNode);
	}

	boost::shared_ptr<LocationNode> MifareChip::getRootLocationNode()
	{
		boost::shared_ptr<LocationNode> rootNode;		
		rootNode.reset(new LocationNode());

		rootNode->setName("Mifare Classic");
		boost::shared_ptr<MifareLocation> rootLocation;
		rootLocation.reset(new MifareLocation());
		rootLocation->sector = (unsigned int)-1;
		rootNode->setLocation(rootLocation);

		if (d_cardprovider)
		{
			for (int i = 0; i < 16; i++)
			{
				addSectorNode(rootNode, i);
			}
		}

		return rootNode;
	}
}

