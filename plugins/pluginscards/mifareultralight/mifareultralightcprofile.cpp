/**
 * \file mifareultralightcprofile.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Mifare Ultralight C card profiles.
 */

#include <logicalaccess/logs.hpp>
#include "mifareultralightcprofile.hpp"

#include "logicalaccess/services/accesscontrol/formats/bithelper.hpp"
#include "mifareultralightlocation.hpp"
#include "mifareultralightcaccessinfo.hpp"

namespace logicalaccess
{
    MifareUltralightCProfile::MifareUltralightCProfile()
        : MifareUltralightProfile()
    {
        clearKeys();
    }

    MifareUltralightCProfile::~MifareUltralightCProfile()
    {
    }

    void MifareUltralightCProfile::setDefaultKeys()
    {
        d_key.reset(new TripleDESKey("49 45 4D 4B 41 45 52 42 21 4E 41 43 55 4F 59 46"));
    }

    void MifareUltralightCProfile::clearKeys()
    {
        setDefaultKeys();
    }

    void MifareUltralightCProfile::setKeyAt(std::shared_ptr<Location> /*location*/, std::shared_ptr<AccessInfo> AccessInfo)
    {
        EXCEPTION_ASSERT_WITH_LOG(AccessInfo, std::invalid_argument, "AccessInfo cannot be null.");

        std::shared_ptr<MifareUltralightCAccessInfo> mAi = std::dynamic_pointer_cast<MifareUltralightCAccessInfo>(AccessInfo);
        EXCEPTION_ASSERT_WITH_LOG(mAi, std::invalid_argument, "AccessInfo must be a MifareUltralightCAccessInfo.");

        d_key = mAi->key;
    }

    void MifareUltralightCProfile::setKey(std::shared_ptr<TripleDESKey> key)
    {
        d_key = key;
    }

    std::shared_ptr<TripleDESKey> MifareUltralightCProfile::getKey() const
    {
        return d_key;
    }

    std::shared_ptr<AccessInfo> MifareUltralightCProfile::createAccessInfo() const
    {
        std::shared_ptr<MifareUltralightCAccessInfo> ret;
        ret.reset(new MifareUltralightCAccessInfo());
        return ret;
    }
}