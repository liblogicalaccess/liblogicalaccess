/**
 * \file topazprofile.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Topaz card profiles.
 */

#include "topazprofile.hpp"

#include "logicalaccess/services/accesscontrol/formats/bithelper.hpp"
#include "topazlocation.hpp"
#include "topazaccessinfo.hpp"

namespace logicalaccess
{
    TopazProfile::TopazProfile()
        : Profile()
    {
    }

    TopazProfile::~TopazProfile()
    {
    }

    void TopazProfile::setDefaultKeys()
    {
    }

    void TopazProfile::setDefaultKeysAt(std::shared_ptr<Location> /*location*/)
    {
    }

    void TopazProfile::clearKeys()
    {
    }

    void TopazProfile::setKeyAt(std::shared_ptr<Location> /*location*/, std::shared_ptr<AccessInfo> /*AccessInfo*/)
    {
    }

    std::shared_ptr<AccessInfo> TopazProfile::createAccessInfo() const
    {
        std::shared_ptr<TopazAccessInfo> ret;
        ret.reset(new TopazAccessInfo());
        return ret;
    }

    std::shared_ptr<Location> TopazProfile::createLocation() const
    {
        std::shared_ptr<TopazLocation> ret;
        ret.reset(new TopazLocation());
        return ret;
    }

    FormatList TopazProfile::getSupportedFormatList()
    {
        FormatList list = Profile::getHIDWiegandFormatList();
        return list;
    }
}