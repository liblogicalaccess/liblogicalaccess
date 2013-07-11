/**
 * \file twicprofile.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Twic card profiles.
 */

#include "twicprofile.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>

#include "twiclocation.hpp"
#include "../iso7816/iso7816location.hpp"
#include "logicalaccess/services/accesscontrol/formats/fascn200bitformat.hpp"

namespace logicalaccess
{
	TwicProfile::TwicProfile()
		: ISO7816Profile()
	{

	}

	TwicProfile::~TwicProfile()
	{

	}

	boost::shared_ptr<Location> TwicProfile::createLocation() const
	{
		boost::shared_ptr<TwicLocation> ret;
		ret.reset(new TwicLocation());
		return ret;
	}

	FormatList TwicProfile::getSupportedFormatList()
	{
		FormatList formats;

		formats.push_back(boost::shared_ptr<FASCN200BitFormat>(new FASCN200BitFormat()));

		return formats;
	}
}