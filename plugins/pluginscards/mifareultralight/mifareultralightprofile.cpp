/**
 * \file mifareultralightprofile.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Mifare Ultralight card profiles.
 */

#include "mifareultralightprofile.hpp"

#include "logicalaccess/services/accesscontrol/formats/bithelper.hpp"
#include "mifareultralightlocation.hpp"
#include "mifareultralightaccessinfo.hpp"

namespace logicalaccess
{
    MifareUltralightProfile::MifareUltralightProfile()
        : Profile()
    {
    }

    MifareUltralightProfile::~MifareUltralightProfile()
    {
    }

    void MifareUltralightProfile::setDefaultKeys()
    {
    }

    void MifareUltralightProfile::setDefaultKeysAt(std::shared_ptr<Location> /*location*/)
    {
    }

    void MifareUltralightProfile::clearKeys()
    {
    }

    void MifareUltralightProfile::setKeyAt(std::shared_ptr<Location> /*location*/, std::shared_ptr<AccessInfo> /*AccessInfo*/)
    {
    }

    std::shared_ptr<AccessInfo> MifareUltralightProfile::createAccessInfo() const
    {
        std::shared_ptr<MifareUltralightAccessInfo> ret;
        ret.reset(new MifareUltralightAccessInfo());
        return ret;
    }

    std::shared_ptr<Location> MifareUltralightProfile::createLocation() const
    {
        std::shared_ptr<MifareUltralightLocation> ret;
        ret.reset(new MifareUltralightLocation());
        return ret;
    }

    FormatList MifareUltralightProfile::getSupportedFormatList()
    {
        FormatList list = Profile::getHIDWiegandFormatList();
        return list;
    }
}