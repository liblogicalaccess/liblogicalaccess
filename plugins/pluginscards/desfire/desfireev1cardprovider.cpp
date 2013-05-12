/**
 * \file desfireev1cardprovider.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief DESFire EV1 card provider.
 */

#include "desfireev1cardprovider.hpp"
#include "desfireev1chip.hpp"

namespace logicalaccess
{
	boost::shared_ptr<DESFireEV1Chip> DESFireEV1CardProvider::getDESFireEV1Chip()
	{
		return boost::dynamic_pointer_cast<DESFireEV1Chip>(getDESFireChip());
	}

	void DESFireEV1CardProvider::selectApplication(boost::shared_ptr<DESFireLocation> location)
	{
		boost::shared_ptr<DESFireEV1Location> evlocation = boost::dynamic_pointer_cast<DESFireEV1Location>(location);
		DESFireCardProvider::selectApplication(location);		
		if (evlocation)
		{
			getDESFireChip()->getDESFireProfile()->setKey(evlocation->aid, 0, getDESFireChip()->getDESFireProfile()->getDefaultKey(evlocation->cryptoMethod));
		}
	}

	void DESFireEV1CardProvider::createApplication(boost::shared_ptr<DESFireLocation> location, DESFireKeySettings settings, int maxNbKeys)
	{
		boost::shared_ptr<DESFireEV1Location> evlocation = boost::dynamic_pointer_cast<DESFireEV1Location>(location);
		if (evlocation)
		{
			if (!getDESFireEV1Commands()->createApplication(location->aid, settings, maxNbKeys, evlocation->useISO7816 ? FIDS_ISO_FID : FIDS_NO_ISO_FID, evlocation->cryptoMethod, evlocation->useISO7816 ? evlocation->applicationFID : 0))
			{
				THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Can't create the application.");
			}
		}
		else
		{
			DESFireCardProvider::createApplication(location, settings, maxNbKeys);
		}
	}

	void DESFireEV1CardProvider::createStdDataFile(boost::shared_ptr<DESFireLocation> location, DESFireAccessRights accessRights, int fileSize)
	{
		boost::shared_ptr<DESFireEV1Location> evlocation = boost::dynamic_pointer_cast<DESFireEV1Location>(location);
		if (evlocation)
		{
			if (!getDESFireEV1Commands()->createStdDataFile(location->file, location->securityLevel, accessRights, fileSize, evlocation->useISO7816 ? evlocation->fileFID : 0))
			{
				THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Can't create the file.");
			}	
		}
		else
		{
			DESFireCardProvider::createStdDataFile(location, accessRights, fileSize);
		}
	}
}
