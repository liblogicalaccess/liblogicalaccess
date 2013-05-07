/**
 * \file ISO15693Chip.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief ISO15693 chip.
 */

#include "ISO15693Chip.h"
#include "ISO15693Profile.h"

#include <iostream>
#include <iomanip>
#include <sstream>

using std::endl;
using std::dec;
using std::hex;
using std::setfill;
using std::setw;
using std::ostringstream;
using std::istringstream;

#include "logicalaccess/Services/AccessControl/Formats/BitHelper.h"


namespace LOGICALACCESS
{
	ISO15693Chip::ISO15693Chip(std::string ct)
		: Chip(ct)
	{
		d_profile.reset(new ISO15693Profile());
	}

	ISO15693Chip::ISO15693Chip()
		: Chip(CHIP_ISO15693)
	{
		d_profile.reset(new ISO15693Profile());
	}

	ISO15693Chip::~ISO15693Chip()
	{

	}	

	boost::shared_ptr<LocationNode> ISO15693Chip::getRootLocationNode()
	{
		boost::shared_ptr<LocationNode> rootNode;
		rootNode.reset(new LocationNode());

		rootNode->setName("ISO 15693");
		rootNode->setHasProperties(true);

		boost::shared_ptr<ISO15693Location> rootLocation;
		rootLocation.reset(new ISO15693Location());
		//FIXME: We need to add a specific property for this
		rootLocation->block = static_cast<int>(-1);
		rootNode->setLocation(rootLocation);	

		if (d_cardprovider)
		{
			ISO15693Commands::SystemInformation sysinfo = getISO15693CardProvider()->getISO15693Commands()->getSystemInformation();
			if (sysinfo.hasVICCMemorySize)
			{
				char tmpName[255];
				for (int i = 0; i < sysinfo.nbBlocks; i++)
				{
					boost::shared_ptr<LocationNode> blockNode;
					blockNode.reset(new LocationNode());

					sprintf(tmpName, "Block %d", i);
					blockNode->setName(tmpName);
					blockNode->setNeedAuthentication(true);
					blockNode->setHasProperties(true);
					blockNode->setLength(sysinfo.blockSize);

					boost::shared_ptr<ISO15693Location> blockLocation;
					blockLocation.reset(new ISO15693Location());
					blockLocation->block = i;
					blockNode->setLocation(blockLocation);				

					blockNode->setParent(rootNode);
					rootNode->getChildrens().push_back(blockNode);
				}
			}
		}

		return rootNode;
	}
}
