/**
 * \file profile.cpp
 * \author Julien K. <julien-dev@islog.com>, Maxime C. <maxime-dev@islog.com>
 * \brief Card profiles.
 */

#include "logicalaccess/cards/profile.hpp"

#include "logicalaccess/services/accesscontrol/formats/wiegand26format.hpp"
#include "logicalaccess/services/accesscontrol/formats/wiegand34format.hpp"
#include "logicalaccess/services/accesscontrol/formats/wiegand34withfacilityformat.hpp"
#include "logicalaccess/services/accesscontrol/formats/wiegand37format.hpp"
#include "logicalaccess/services/accesscontrol/formats/wiegand37withfacilityformat.hpp"
#include "logicalaccess/services/accesscontrol/formats/corporate1000format.hpp"
#include "logicalaccess/services/accesscontrol/formats/dataclockformat.hpp"

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