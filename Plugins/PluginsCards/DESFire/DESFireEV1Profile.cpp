/**
 * \file DESFireEV1Profile.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief DESFire EV1 card profiles.
 */

#include "DESFireEV1Profile.h"


#include <cstring>

namespace LOGICALACCESS
{
	DESFireEV1Profile::DESFireEV1Profile() : 
		DESFireProfile()
	{
		
	}

	DESFireEV1Profile::~DESFireEV1Profile()
	{

	}

	void DESFireEV1Profile::setDefaultKeysAt(boost::shared_ptr<Location> location)
	{
		EXCEPTION_ASSERT_WITH_LOG(location, std::invalid_argument, "location cannot be null.");

		boost::shared_ptr<DESFireEV1Location> dfLocation = boost::dynamic_pointer_cast<DESFireEV1Location>(location);
		EXCEPTION_ASSERT_WITH_LOG(dfLocation, std::invalid_argument, "location must be a DESFireEV1Location.");

		// Application (File keys are Application keys)
		if (dfLocation->aid != -1)
		{
			for (unsigned char i = 0; i < 14; ++i)
			{
				setKey(dfLocation->aid, i, getDefaultKey(dfLocation->cryptoMethod));
			}
		}
		// Card
		else
		{
			setKey(0, 0, getDefaultKey(DF_KEY_DES));
		}
	}

	boost::shared_ptr<Location> DESFireEV1Profile::createLocation() const
	{
		boost::shared_ptr<DESFireEV1Location> ret;
		ret.reset(new DESFireEV1Location());
		return ret;
	}
}
