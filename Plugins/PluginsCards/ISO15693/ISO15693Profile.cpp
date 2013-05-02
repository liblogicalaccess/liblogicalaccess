/**
 * \file ISO15693Profile.cpp
 * \author Maxime CHAMLEY <maxime.chamley@islog.eu>
 * \brief ISO15693 card profiles.
 */

#include "ISO15693Profile.h"

namespace LOGICALACCESS
{	
	ISO15693Profile::ISO15693Profile()
		: Profile()
	{

	}

	ISO15693Profile::~ISO15693Profile()
	{

	}

	void ISO15693Profile::setDefaultKeys()
	{

	}

	void ISO15693Profile::clearKeys()
	{

	}

	void ISO15693Profile::setDefaultKeysAt(boost::shared_ptr<Location> /*location*/)
	{
	}
		
	void ISO15693Profile::setKeyAt(boost::shared_ptr<Location> /*location*/, boost::shared_ptr<AccessInfo> /*AccessInfo*/)
	{
	}

	boost::shared_ptr<AccessInfo> ISO15693Profile::createAccessInfo() const
	{
		boost::shared_ptr<AccessInfo> ret;
		return ret;
	}

	boost::shared_ptr<Location> ISO15693Profile::createLocation() const
	{
		boost::shared_ptr<ISO15693Location> ret;
		ret.reset(new ISO15693Location());
		return ret;
	}

	FormatList ISO15693Profile::getSupportedFormatList()
	{
		return Profile::getHIDWiegandFormatList();
	}
}
