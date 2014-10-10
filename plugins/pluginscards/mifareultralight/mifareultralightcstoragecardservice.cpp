/**
 * \file mifareultralightcstoragecardservice.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Mifare Ultralight C storage card service.
 */

#include "mifareultralightcstoragecardservice.hpp"
#include "mifareultralightcchip.hpp"

namespace logicalaccess
{
    MifareUltralightCStorageCardService::MifareUltralightCStorageCardService(boost::shared_ptr<Chip> chip)
        : MifareUltralightStorageCardService(chip)
    {
    }

    MifareUltralightCStorageCardService::~MifareUltralightCStorageCardService()
    {
    }

    void MifareUltralightCStorageCardService::erase()
    {
        getMifareUltralightCChip()->getMifareUltralightCCommands()->authenticate(getMifareUltralightCChip()->getMifareUltralightCProfile()->getKey());

        MifareUltralightStorageCardService::erase();
    }

    void MifareUltralightCStorageCardService::erase(boost::shared_ptr<Location> location, boost::shared_ptr<AccessInfo> aiToUse)
    {
        getMifareUltralightCChip()->getMifareUltralightCCommands()->authenticate(aiToUse);

        MifareUltralightStorageCardService::erase(location, aiToUse);
    }

    void MifareUltralightCStorageCardService::writeData(boost::shared_ptr<Location> location, boost::shared_ptr<AccessInfo> aiToUse, boost::shared_ptr<AccessInfo> aiToWrite, const void* data, size_t dataLength, CardBehavior behaviorFlags)
    {
        getMifareUltralightCChip()->getMifareUltralightCCommands()->authenticate(aiToUse);

        MifareUltralightStorageCardService::writeData(location, aiToUse, aiToWrite, data, dataLength, behaviorFlags);

        if (aiToWrite)
        {
            getMifareUltralightCChip()->getMifareUltralightCCommands()->changeKey(aiToWrite);
        }
    }

    void MifareUltralightCStorageCardService::readData(boost::shared_ptr<Location> location, boost::shared_ptr<AccessInfo> aiToUse, void* data, size_t dataLength, CardBehavior behaviorFlags)
    {
        getMifareUltralightCChip()->getMifareUltralightCCommands()->authenticate(aiToUse);

        MifareUltralightStorageCardService::readData(location, aiToUse, data, dataLength, behaviorFlags);
    }
}