/**
 * \file felicaprofile.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief FeliCa card profiles.
 */

#include "felicaprofile.hpp"

namespace logicalaccess
{
	FeliCaProfile::FeliCaProfile()
        : Profile()
    {
    }

	FeliCaProfile::~FeliCaProfile()
    {
    }

	void FeliCaProfile::setDefaultKeys()
    {
    }

	void FeliCaProfile::clearKeys()
    {
    }

	void FeliCaProfile::setDefaultKeysAt(std::shared_ptr<Location> /*location*/)
    {
    }

	void FeliCaProfile::setKeyAt(std::shared_ptr<Location> /*location*/, std::shared_ptr<AccessInfo> /*AccessInfo*/)
    {
    }

	std::shared_ptr<AccessInfo> FeliCaProfile::createAccessInfo() const
    {
        std::shared_ptr<AccessInfo> ret;
        return ret;
    }

	std::shared_ptr<Location> FeliCaProfile::createLocation() const
    {
		std::shared_ptr<FeliCaLocation> ret;
		ret.reset(new FeliCaLocation());
        return ret;
    }

	FormatList FeliCaProfile::getSupportedFormatList()
    {
        return Profile::getHIDWiegandFormatList();
    }
}