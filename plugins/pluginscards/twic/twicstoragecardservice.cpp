/**
 * \file twicstoragecardservice.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Twic storage card service.
 */

#include "twicstoragecardservice.hpp"

namespace logicalaccess
{
    TwicStorageCardService::TwicStorageCardService(std::shared_ptr<Chip> chip)
        : ISO7816StorageCardService(chip)
    {
    }

    TwicStorageCardService::~TwicStorageCardService()
    {
    }

    void TwicStorageCardService::readData(std::shared_ptr<Location> location, std::shared_ptr<AccessInfo> aiToUse, void *data, size_t dataLength, CardBehavior behaviorFlags)
    {
        EXCEPTION_ASSERT_WITH_LOG(location, std::invalid_argument, "location cannot be null.");

        std::shared_ptr<ISO7816Location> icISOLocation = std::dynamic_pointer_cast<ISO7816Location>(location);
        std::shared_ptr<TwicLocation> icLocation = std::dynamic_pointer_cast<TwicLocation>(location);

        if (icISOLocation)
        {
            ISO7816StorageCardService::readData(location, aiToUse, data, dataLength, behaviorFlags);
            return;
        }
        EXCEPTION_ASSERT_WITH_LOG(icLocation, std::invalid_argument, "location must be a TwicLocation or ISO7816Location.");

        getTwicChip()->getTwicCommands()->selectTWICApplication();
        if (icLocation->tag == 0x00)
        {
            std::vector<unsigned char> result = getTwicChip()->getTwicCommands()->getTWICData(icLocation->dataObject);
            unsigned char *buf = (unsigned char*)data;
            for (unsigned char x = 0; x < dataLength && x < result.size(); ++x)
                buf[x] = result[x];
        }
        else
        {
            // A tag is specified, the user want to get only the tag's data.
            size_t dataObjectLength = getTwicChip()->getTwicCommands()->getDataObjectLength(icLocation->dataObject, true);
            std::vector<unsigned char> fulldata = getTwicChip()->getTwicCommands()->getTWICData(icLocation->dataObject);

            if (fulldata.size())
            {
                size_t offset = getTwicChip()->getTwicCommands()->getMinimumBytesRepresentation(getTwicChip()->getTwicCommands()->getMaximumDataObjectLength(icLocation->dataObject)) + 1;
                if (offset < dataObjectLength)
                {
                    getTwicChip()->getTwicCommands()->getTagData(icLocation, &fulldata[offset], dataObjectLength - offset, data, dataLength);
                }
            }
        }
    }
}