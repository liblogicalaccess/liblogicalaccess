/**
 * \file Profile.cpp
 * \author Julien K. <julien-dev@islog.com>, Maxime C. <maxime-dev@islog.com>
 * \brief Card profiles.
 */

#include "logicalaccess/Cards/Profile.h"

#include "logicalaccess/Services/AccessControl/Formats/Wiegand26Format.h"
#include "logicalaccess/Services/AccessControl/Formats/Wiegand34Format.h"
#include "logicalaccess/Services/AccessControl/Formats/Wiegand34WithFacilityFormat.h"
#include "logicalaccess/Services/AccessControl/Formats/Wiegand37Format.h"
#include "logicalaccess/Services/AccessControl/Formats/Wiegand37WithFacilityFormat.h"
#include "logicalaccess/Services/AccessControl/Formats/Corporate1000Format.h"
#include "logicalaccess/Services/AccessControl/Formats/DataClockFormat.h"

#include <fstream>

using std::ofstream;
using std::ifstream;


namespace logicalaccess
{
	Profile::Profile()
	{
	}

	FormatList Profile::getHIDWiegandFormatList()
	{
		FormatList formats;

		formats.push_back(boost::shared_ptr<Wiegand26Format>(new Wiegand26Format()));
		formats.push_back(boost::shared_ptr<Wiegand34Format>(new Wiegand34Format()));
		formats.push_back(boost::shared_ptr<Wiegand34WithFacilityFormat>(new Wiegand34WithFacilityFormat()));
		formats.push_back(boost::shared_ptr<Wiegand37Format>(new Wiegand37Format()));
		formats.push_back(boost::shared_ptr<Wiegand37WithFacilityFormat>(new Wiegand37WithFacilityFormat()));
		formats.push_back(boost::shared_ptr<DataClockFormat>(new DataClockFormat()));
		formats.push_back(boost::shared_ptr<Corporate1000Format>(new Corporate1000Format()));

		return formats;
	}
}

