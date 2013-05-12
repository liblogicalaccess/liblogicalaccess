/**
 * \file proxprofile.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Prox card profiles.
 */

#include "proxprofile.hpp"

#include "logicalaccess/services/accesscontrol/formats/bithelper.hpp"
#include "proxlocation.hpp"

#include "logicalaccess/services/accesscontrol/formats/wiegand26format.hpp"
#include "logicalaccess/services/accesscontrol/formats/wiegand34format.hpp"
#include "logicalaccess/services/accesscontrol/formats/wiegand34withfacilityformat.hpp"
#include "logicalaccess/services/accesscontrol/formats/wiegand37format.hpp"
#include "logicalaccess/services/accesscontrol/formats/wiegand37withfacilityformat.hpp"
#include "logicalaccess/services/accesscontrol/formats/rawformat.hpp"

namespace logicalaccess
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
