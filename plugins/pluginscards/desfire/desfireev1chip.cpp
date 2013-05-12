/**
 * \file desfireev1chip.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief DESFire EV1 chip.
 */

#include "desfireev1chip.hpp"

#include <cstring>

namespace logicalaccess
{
	DESFireEV1Chip::DESFireEV1Chip() : 
		DESFireChip(CHIP_DESFIRE_EV1)
	{
		d_profile.reset(new DESFireEV1Profile());
	}

	DESFireEV1Chip::~DESFireEV1Chip()
	{

	}

	boost::shared_ptr<LocationNode> DESFireEV1Chip::getRootLocationNode()
	{
		boost::shared_ptr<LocationNode> rootNode = DESFireChip::getRootLocationNode();
		rootNode->setName("Mifare DESFire EV1");
		
		return rootNode;
	}

	boost::shared_ptr<DESFireLocation> DESFireEV1Chip::getApplicationLocation()
	{
		boost::shared_ptr<DESFireEV1Location> location(new DESFireEV1Location());

		try
		{
			DESFireKeySettings keySettings;
			unsigned int maxNbkeys = 0;
			DESFireKeyType keyType;
			getDESFireEV1CardProvider()->getDESFireEV1Commands()->getKeySettings(keySettings, maxNbkeys, keyType);

			if (keyType != DF_KEY_DES)
			{
				location->useEV1 = true;
				location->cryptoMethod = keyType;
			}
		}
		catch(std::exception&)
		{
		}

		return location;
	}
}
