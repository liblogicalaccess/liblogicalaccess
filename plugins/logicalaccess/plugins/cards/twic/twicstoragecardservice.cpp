/**
 * \file twicstoragecardservice.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Twic storage card service.
 */

#include <logicalaccess/plugins/llacommon/logs.hpp>
#include <logicalaccess/plugins/cards/twic/twicstoragecardservice.hpp>

namespace logicalaccess
{
TwicStorageCardService::TwicStorageCardService(std::shared_ptr<Chip> chip)
    : ISO7816StorageCardService(chip)
{
}

TwicStorageCardService::~TwicStorageCardService()
{
}

ByteVector TwicStorageCardService::readData(std::shared_ptr<Location> location,
                                            std::shared_ptr<AccessInfo> aiToUse,
                                            size_t length, CardBehavior behaviorFlags)
{
    ByteVector result;
    EXCEPTION_ASSERT_WITH_LOG(location, std::invalid_argument,
                              "location cannot be null.");

    std::shared_ptr<ISO7816Location> icISOLocation =
        std::dynamic_pointer_cast<ISO7816Location>(location);
    std::shared_ptr<TwicLocation> icLocation =
        std::dynamic_pointer_cast<TwicLocation>(location);

    if (icISOLocation)
    {
        return ISO7816StorageCardService::readData(location, aiToUse, length,
                                                   behaviorFlags);
    }
    EXCEPTION_ASSERT_WITH_LOG(icLocation, std::invalid_argument,
                              "location must be a TwicLocation or ISO7816Location.");

    getTwicChip()->getTwicCommands()->selectTWICApplication();
    if (icLocation->tag == 0x00)
    {
        result = getTwicChip()->getTwicCommands()->getISO7816Commands()->getDataList(
            icLocation->dataObject, 0);
    }
    else
    {
        // A tag is specified, the user want to get only the tag's data.
        size_t dataObjectLength = getTwicChip()->getTwicCommands()->getDataObjectLength(
            icLocation->dataObject, true);
        ByteVector fulldata =
            getTwicChip()->getTwicCommands()->getISO7816Commands()->getDataList(
                icLocation->dataObject, 0);

        if (fulldata.size())
        {
            size_t offset =
                getTwicChip()->getTwicCommands()->getMinimumBytesRepresentation(
                    getTwicChip()->getTwicCommands()->getMaximumDataObjectLength(
                        icLocation->dataObject)) +
                1;
            if (offset < dataObjectLength)
            {
                result      = ByteVector(256);
                size_t size = 256;
                getTwicChip()->getTwicCommands()->getTagData(
                    icLocation, &fulldata[offset], dataObjectLength - offset, &result[0],
                    size);
                result.resize(size);
            }
        }
    }
    return result;
}
}