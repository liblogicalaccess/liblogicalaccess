/**
 * \file cps3profile.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief CPS3 card profiles.
 */

#include "cps3profile.hpp"

namespace logicalaccess
{
	CPS3Profile::CPS3Profile()
        : ISO7816Profile()
    {
    }

	CPS3Profile::~CPS3Profile()
    {
    }

	std::shared_ptr<Location> CPS3Profile::createLocation() const
    {
		std::shared_ptr<CPS3Location> ret;
		ret.reset(new CPS3Location());
        return ret;
    }

	FormatList CPS3Profile::getSupportedFormatList()
    {
        return Profile::getHIDWiegandFormatList();
    }
}