/**
 * \file mifareultralightcstoragecardservice.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Mifare Ultralight C storage card service.
 */

#include <logicalaccess/plugins/cards/mifareultralight/mifareultralightcstoragecardservice.hpp>
#include <logicalaccess/plugins/cards/mifareultralight/mifareultralightcchip.hpp>

namespace logicalaccess
{
MifareUltralightCStorageCardService::MifareUltralightCStorageCardService(
    std::shared_ptr<Chip> chip)
    : MifareUltralightStorageCardService(chip)
{
}

MifareUltralightCStorageCardService::~MifareUltralightCStorageCardService()
{
}

void MifareUltralightCStorageCardService::erase(std::shared_ptr<Location> location,
                                                std::shared_ptr<AccessInfo> aiToUse)
{
    getMifareUltralightCChip()->getMifareUltralightCCommands()->authenticate(aiToUse);

    MifareUltralightStorageCardService::erase(location, aiToUse);
}

void MifareUltralightCStorageCardService::writeData(std::shared_ptr<Location> location,
                                                    std::shared_ptr<AccessInfo> aiToUse,
                                                    std::shared_ptr<AccessInfo> aiToWrite,
                                                    const ByteVector &data,
                                                    CardBehavior behaviorFlags)
{
    getMifareUltralightCChip()->getMifareUltralightCCommands()->authenticate(aiToUse);

    MifareUltralightStorageCardService::writeData(location, aiToUse, aiToWrite, data,
                                                  behaviorFlags);

    if (aiToWrite)
    {
        getMifareUltralightCChip()->getMifareUltralightCCommands()->changeKey(aiToWrite);
    }
}

ByteVector
MifareUltralightCStorageCardService::readData(std::shared_ptr<Location> location,
                                              std::shared_ptr<AccessInfo> aiToUse,
                                              size_t length, CardBehavior behaviorFlags)
{
    getMifareUltralightCChip()->getMifareUltralightCCommands()->authenticate(aiToUse);

    return MifareUltralightStorageCardService::readData(location, aiToUse, length,
                                                        behaviorFlags);
}
}