/**
 * \file iso7816profile.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief ISO7816 card profiles.
 */

#include "iso7816profile.hpp"

namespace logicalaccess
{
    ISO7816Profile::ISO7816Profile()
        : Profile()
    {
    }

    ISO7816Profile::~ISO7816Profile()
    {
    }

    void ISO7816Profile::setDefaultKeys()
    {
    }

    void ISO7816Profile::clearKeys()
    {
    }

    void ISO7816Profile::setDefaultKeysAt(boost::shared_ptr<Location> /*location*/)
    {
    }

    void ISO7816Profile::setKeyAt(boost::shared_ptr<Location> /*location*/, boost::shared_ptr<AccessInfo> /*AccessInfo*/)
    {
    }

    boost::shared_ptr<AccessInfo> ISO7816Profile::createAccessInfo() const
    {
        boost::shared_ptr<AccessInfo> ret;
        return ret;
    }

    boost::shared_ptr<Location> ISO7816Profile::createLocation() const
    {
        boost::shared_ptr<ISO7816Location> ret;
        ret.reset(new ISO7816Location());
        return ret;
    }

    FormatList ISO7816Profile::getSupportedFormatList()
    {
        return Profile::getHIDWiegandFormatList();
    }
}