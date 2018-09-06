/**
 * \file iso15693storagecardservice.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief ISO15693 storage card service.
 */

#include <logicalaccess/plugins/llacommon/logs.hpp>
#include <logicalaccess/plugins/cards/iso15693/iso15693storagecardservice.hpp>
#include <logicalaccess/plugins/cards/iso15693/iso15693location.hpp>
#include <logicalaccess/cards/locationnode.hpp>

namespace logicalaccess
{
ISO15693StorageCardService::ISO15693StorageCardService(std::shared_ptr<Chip> chip)
    : StorageCardService(chip)
{
}

ISO15693StorageCardService::~ISO15693StorageCardService()
{
}

void ISO15693StorageCardService::erase(std::shared_ptr<Location> location,
                                       std::shared_ptr<AccessInfo> aiToUse)
{
    ISO15693Commands::SystemInformation sysinfo =
        getISO15693Chip()->getISO15693Commands()->getSystemInformation();

    if (sysinfo.hasVICCMemorySize)
    {
        ByteVector tmp(sysinfo.blockSize, 0x00);

        std::shared_ptr<AccessInfo> ai;
        writeData(location, aiToUse, ai, tmp, CB_DEFAULT);
    }
}

void ISO15693StorageCardService::writeData(std::shared_ptr<Location> location,
                                           std::shared_ptr<AccessInfo>,
                                           std::shared_ptr<AccessInfo>,
                                           const ByteVector &data, CardBehavior)
{
    EXCEPTION_ASSERT_WITH_LOG(location, std::invalid_argument,
                              "location cannot be null.");

    std::shared_ptr<ISO15693Location> icLocation =
        std::dynamic_pointer_cast<ISO15693Location>(location);

    EXCEPTION_ASSERT_WITH_LOG(icLocation, std::invalid_argument,
                              "location must be a ISO15693Location.");

    getISO15693Chip()->getISO15693Commands()->writeBlock(icLocation->block, data);
}

ByteVector ISO15693StorageCardService::readData(std::shared_ptr<Location> location,
                                                std::shared_ptr<AccessInfo>,
                                                size_t /*length*/, CardBehavior)
{
    EXCEPTION_ASSERT_WITH_LOG(location, std::invalid_argument,
                              "location cannot be null.");

    std::shared_ptr<ISO15693Location> icLocation =
        std::dynamic_pointer_cast<ISO15693Location>(location);

    EXCEPTION_ASSERT_WITH_LOG(icLocation, std::invalid_argument,
                              "location must be a ISO15693Location.");

    return getISO15693Chip()->getISO15693Commands()->readBlock(icLocation->block);
}

ByteVector
    ISO15693StorageCardService::readDataHeader(std::shared_ptr<Location> /*location*/,
                                               std::shared_ptr<AccessInfo> /*aiToUse*/)
{
    return {};
}
}