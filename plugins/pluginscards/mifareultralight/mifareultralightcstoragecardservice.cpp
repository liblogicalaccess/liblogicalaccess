/**
 * \file mifareultralightcstoragecardservice.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Mifare Ultralight C storage card service.
 */

#include "mifareultralightcstoragecardservice.hpp"
#include "mifareultralightcchip.hpp"

namespace logicalaccess
{
    MifareUltralightCStorageCardService::MifareUltralightCStorageCardService(std::shared_ptr<Chip> chip)
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

    void MifareUltralightCStorageCardService::erase(std::shared_ptr<Location> location, std::shared_ptr<AccessInfo> aiToUse)
    {
        getMifareUltralightCChip()->getMifareUltralightCCommands()->authenticate(aiToUse);

        MifareUltralightStorageCardService::erase(location, aiToUse);
    }

    void MifareUltralightCStorageCardService::writeData(std::shared_ptr<Location> location, std::shared_ptr<AccessInfo> aiToUse, std::shared_ptr<AccessInfo> aiToWrite, const void* data, size_t dataLength, CardBehavior behaviorFlags)
    {
        getMifareUltralightCChip()->getMifareUltralightCCommands()->authenticate(aiToUse);

        MifareUltralightStorageCardService::writeData(location, aiToUse, aiToWrite, data, dataLength, behaviorFlags);

        if (aiToWrite)
        {
            getMifareUltralightCChip()->getMifareUltralightCCommands()->changeKey(aiToWrite);
        }
    }

    void MifareUltralightCStorageCardService::readData(std::shared_ptr<Location> location, std::shared_ptr<AccessInfo> aiToUse, void* data, size_t dataLength, CardBehavior behaviorFlags)
    {
        getMifareUltralightCChip()->getMifareUltralightCCommands()->authenticate(aiToUse);

        MifareUltralightStorageCardService::readData(location, aiToUse, data, dataLength, behaviorFlags);
    }
}