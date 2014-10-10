/**
 * \file iso15693storagecardservice.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief ISO15693 storage card service.
 */

#include "iso15693storagecardservice.hpp"
#include "iso15693location.hpp"

namespace logicalaccess
{
    ISO15693StorageCardService::ISO15693StorageCardService(boost::shared_ptr<Chip> chip)
        : StorageCardService(chip)
    {
    }

    ISO15693StorageCardService::~ISO15693StorageCardService()
    {
    }

    void ISO15693StorageCardService::erase()
    {
        boost::shared_ptr<LocationNode> rootNode = getChip()->getRootLocationNode();
        std::vector<boost::shared_ptr<LocationNode> > childs = rootNode->getChildrens();

        for (size_t i = 0; i < childs.size(); ++i)
        {
            boost::shared_ptr<LocationNode> blockNode = childs.at(i);
            boost::shared_ptr<AccessInfo> ai;
            erase(blockNode->getLocation(), ai);
        }
    }

    void ISO15693StorageCardService::erase(boost::shared_ptr<Location> location, boost::shared_ptr<AccessInfo> aiToUse)
    {
        ISO15693Commands::SystemInformation sysinfo = getISO15693Chip()->getISO15693Commands()->getSystemInformation();

        if (sysinfo.hasVICCMemorySize)
        {
            unsigned char* tmp = new unsigned char[sysinfo.blockSize];
            memset(tmp, 0x00, sysinfo.blockSize);

            boost::shared_ptr<AccessInfo> ai;
            writeData(location, aiToUse, ai, tmp, sysinfo.blockSize, CB_DEFAULT);
        }
    }

    void ISO15693StorageCardService::writeData(boost::shared_ptr<Location> location, boost::shared_ptr<AccessInfo>, boost::shared_ptr<AccessInfo>, const void* data, size_t dataLength, CardBehavior)
    {
        EXCEPTION_ASSERT_WITH_LOG(location, std::invalid_argument, "location cannot be null.");
        EXCEPTION_ASSERT_WITH_LOG(data, std::invalid_argument, "data cannot be null.");

        boost::shared_ptr<ISO15693Location> icLocation = boost::dynamic_pointer_cast<ISO15693Location>(location);

        EXCEPTION_ASSERT_WITH_LOG(icLocation, std::invalid_argument, "location must be a ISO15693Location.");

        getISO15693Chip()->getISO15693Commands()->writeBlock(icLocation->block, data, dataLength);
    }

    void ISO15693StorageCardService::readData(boost::shared_ptr<Location> location, boost::shared_ptr<AccessInfo>, void* data, size_t dataLength, CardBehavior)
    {
        EXCEPTION_ASSERT_WITH_LOG(location, std::invalid_argument, "location cannot be null.");
        EXCEPTION_ASSERT_WITH_LOG(data, std::invalid_argument, "data cannot be null.");

        boost::shared_ptr<ISO15693Location> icLocation = boost::dynamic_pointer_cast<ISO15693Location>(location);

        EXCEPTION_ASSERT_WITH_LOG(icLocation, std::invalid_argument, "location must be a ISO15693Location.");

        getISO15693Chip()->getISO15693Commands()->readBlock(icLocation->block, data, dataLength, dataLength);
    }

    unsigned int ISO15693StorageCardService::readDataHeader(boost::shared_ptr<Location>, boost::shared_ptr<AccessInfo>, void*, size_t)
    {
        return 0;
    }
}