/**
 * \file desfireev1profile.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief DESFire EV1 card profiles.
 */

#include "desfireev1profile.hpp"


#include <cstring>

namespace logicalaccess
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

		boost::shared_ptr<DESFireEV1Location> dfEV1Location = boost::dynamic_pointer_cast<DESFireEV1Location>(location);
		boost::shared_ptr<DESFireLocation> dfLocation = boost::dynamic_pointer_cast<DESFireLocation>(location);

		if (!dfEV1Location)
			WARNING_SIMPLE_("location is not a DESFireEV1Location.");

		if (dfLocation)
			WARNING_SIMPLE_("DESFireEV1Profile use DESFireLocation so we force DES Crypto.");
		else if (!dfEV1Location)
			EXCEPTION_ASSERT_WITH_LOG(dfEV1Location, std::invalid_argument, "location must be a DESFireEV1Location or DESFireLocation.");

		// Application (File keys are Application keys)
		if (dfLocation->aid != (unsigned int)-1)
		{
			for (unsigned char i = 0; i < 14; ++i)
			{
				if (dfEV1Location)
					setKey(dfEV1Location->aid, i, getDefaultKey(dfEV1Location->cryptoMethod));
				else
					setKey(dfLocation->aid, i, getDefaultKey(DF_KEY_DES));
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
