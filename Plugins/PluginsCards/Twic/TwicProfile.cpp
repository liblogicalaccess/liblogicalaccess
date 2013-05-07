/**
 * \file TwicProfile.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Twic card profiles.
 */

#include "TwicProfile.h"

#include <iostream>
#include <iomanip>
#include <sstream>

using std::endl;
using std::dec;
using std::hex;
using std::setfill;
using std::setw;
using std::ostringstream;
using std::istringstream;

#include "logicalaccess/Services/AccessControl/Formats/BitHelper.h"
#include "../ISO7816/ISO7816Location.h"
#include "logicalaccess/Services/AccessControl/Formats/FASCN200BitFormat.h"

namespace LOGICALACCESS
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