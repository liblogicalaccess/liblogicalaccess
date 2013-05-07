/**
 * \file ProxProfile.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Prox card profiles.
 */

#include "ProxProfile.h"

#include "logicalaccess/Services/AccessControl/Formats/BitHelper.h"
#include "ProxLocation.h"

#include "logicalaccess/Services/AccessControl/Formats/Wiegand26Format.h"
#include "logicalaccess/Services/AccessControl/Formats/Wiegand34Format.h"
#include "logicalaccess/Services/AccessControl/Formats/Wiegand34WithFacilityFormat.h"
#include "logicalaccess/Services/AccessControl/Formats/Wiegand37Format.h"
#include "logicalaccess/Services/AccessControl/Formats/Wiegand37WithFacilityFormat.h"
#include "logicalaccess/Services/AccessControl/Formats/RawFormat.h"

namespace LOGICALACCESS
{
	ProxProfile::ProxProfile()
		: Profile()
	{
	}

	ProxProfile::~ProxProfile()
	{
	}

	void ProxProfile::setDefaultKeys()
	{
	}

	void ProxProfile::clearKeys()
	{
	}

	void ProxProfile::setDefaultKeysAt(boost::shared_ptr<Location> /*location*/)
	{
	}

	void ProxProfile::setKeyAt(boost::shared_ptr<Location> /*location*/, boost::shared_ptr<AccessInfo> /*AccessInfo*/)
	{
	}

	boost::shared_ptr<AccessInfo> ProxProfile::createAccessInfo() const
	{
		boost::shared_ptr<AccessInfo> ret;
		return ret;
	}

	boost::shared_ptr<Location> ProxProfile::createLocation() const
	{
		boost::shared_ptr<ProxLocation> ret;
		ret.reset(new ProxLocation());
		return ret;
	}

	FormatList ProxProfile::getSupportedFormatList()
	{
		FormatList list = Profile::getHIDWiegandFormatList();
		list.push_back(boost::shared_ptr<RawFormat>(new RawFormat()));
		return list;
	}
}
