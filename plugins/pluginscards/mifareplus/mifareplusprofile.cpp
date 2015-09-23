/**
 * \file mifareplusprofile.cpp
 * \author Xavier SCHMERBER <xavier.schmerber@gmail.com>
 * \brief MifarePlus card profiles.
 */

#include "mifareplusprofile.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>
#include <logicalaccess/logs.hpp>

namespace logicalaccess
{
    MifarePlusProfile_OLD::MifarePlusProfile_OLD()
        : Profile()
    {
    }

    MifarePlusProfile_OLD::~MifarePlusProfile_OLD()
    {
    }

    unsigned int MifarePlusProfile_OLD::getNbSectors() const
    {
        return MIFARE_PLUS_MAX_SECTORNB;
    }

    void MifarePlusProfile_OLD::setDefaultKeysAt(std::shared_ptr<Location> location)
    {
        EXCEPTION_ASSERT_WITH_LOG(location, std::invalid_argument, "location cannot be null.");

        std::shared_ptr<MifarePlusLocation> mLocation = std::dynamic_pointer_cast<MifarePlusLocation>(location);
        EXCEPTION_ASSERT_WITH_LOG(mLocation, std::invalid_argument, "location must be a MifarePlusLocation.");

        if (mLocation->sector != -1)
        {
            setDefaultKeysAt(mLocation->sector);
        }
    }

    std::shared_ptr<AccessInfo> MifarePlusProfile_OLD::createAccessInfo() const
    {
        std::shared_ptr<MifarePlusAccessInfo_OLD> ret;
        ret.reset(new MifarePlusAccessInfo_OLD(MIFARE_PLUS_AES_KEY_SIZE));
        return ret;
    }

    std::shared_ptr<Location> MifarePlusProfile_OLD::createLocation() const
    {
        std::shared_ptr<MifarePlusLocation> ret;
        ret.reset(new MifarePlusLocation());
        return ret;
    }

    FormatList MifarePlusProfile_OLD::getSupportedFormatList()
    {
        return Profile::getHIDWiegandFormatList();
    }
}