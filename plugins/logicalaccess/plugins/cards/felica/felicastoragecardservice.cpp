/**
 * \file felicastoragecardservice.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief FeliCa storage card service.
 */

#include <logicalaccess/plugins/llacommon/logs.hpp>
#include <logicalaccess/plugins/cards/felica/felicastoragecardservice.hpp>
#include <logicalaccess/plugins/cards/felica/felicalocation.hpp>
#include <logicalaccess/cards/locationnode.hpp>

namespace logicalaccess
{
FeliCaStorageCardService::FeliCaStorageCardService(std::shared_ptr<Chip> chip)
    : StorageCardService(chip)
{
}

FeliCaStorageCardService::~FeliCaStorageCardService()
{
}

void FeliCaStorageCardService::erase(std::shared_ptr<Location> location,
                                     std::shared_ptr<AccessInfo> /*aiToUse*/)
{
    EXCEPTION_ASSERT_WITH_LOG(location, std::invalid_argument,
                              "location cannot be null.");

    std::shared_ptr<FeliCaLocation> flocation =
        std::dynamic_pointer_cast<FeliCaLocation>(location);

    EXCEPTION_ASSERT_WITH_LOG(flocation, std::invalid_argument,
                              "location must be a FeliCaLocation.");

    std::shared_ptr<FeliCaCommands> cmd = getFeliCaChip()->getFeliCaCommands();
    EXCEPTION_ASSERT_WITH_LOG(cmd, CardException,
                              "FeliCa commands not implemented on this reader.");

    std::vector<unsigned short> blocks;
    for (unsigned short i = flocation->block; i < 14; ++i)
    {
        blocks.push_back(i);
    }
    ByteVector data(blocks.size() * 16, 0x00);
    cmd->write(flocation->code, blocks, data);
}

void FeliCaStorageCardService::writeData(std::shared_ptr<Location> location,
                                         std::shared_ptr<AccessInfo>,
                                         std::shared_ptr<AccessInfo>,
                                         const ByteVector &data,
                                         CardBehavior cardBehavior)
{
    EXCEPTION_ASSERT_WITH_LOG(location, std::invalid_argument,
                              "location cannot be null.");

    std::shared_ptr<FeliCaLocation> icLocation =
        std::dynamic_pointer_cast<FeliCaLocation>(location);

    EXCEPTION_ASSERT_WITH_LOG(icLocation, std::invalid_argument,
                              "location must be a FeliCaLocation.");

    std::shared_ptr<FeliCaCommands> cmd = getFeliCaChip()->getFeliCaCommands();
    EXCEPTION_ASSERT_WITH_LOG(cmd, CardException,
                              "FeliCa commands not implemented on this reader.");

    if ((cardBehavior & CB_AUTOSWITCHAREA) == CB_AUTOSWITCHAREA)
    {
        unsigned int i = 0;
        while (i < data.size())
        {
            cmd->write(icLocation->code, icLocation->block + (unsigned int)(i / 16),
                       ByteVector(data.cbegin() + i, (i + 16 < data.size())
                                                         ? data.begin() + i + 16
                                                         : data.cend()));
            i += 16;
        }
    }
    else
    {
        cmd->write(icLocation->code, icLocation->block, data);
    }
}

ByteVector FeliCaStorageCardService::readData(std::shared_ptr<Location> location,
                                              std::shared_ptr<AccessInfo>, size_t length,
                                              CardBehavior cardBehavior)
{
    EXCEPTION_ASSERT_WITH_LOG(location, std::invalid_argument,
                              "location cannot be null.");

    std::shared_ptr<FeliCaLocation> icLocation =
        std::dynamic_pointer_cast<FeliCaLocation>(location);

    EXCEPTION_ASSERT_WITH_LOG(icLocation, std::invalid_argument,
                              "location must be a FeliCaLocation.");

    std::shared_ptr<FeliCaCommands> cmd = getFeliCaChip()->getFeliCaCommands();
    EXCEPTION_ASSERT_WITH_LOG(cmd, CardException,
                              "FeliCa commands not implemented on this reader.");

    ByteVector data;
    if ((cardBehavior & CB_AUTOSWITCHAREA) == CB_AUTOSWITCHAREA)
    {
        size_t i = 0;
        while (i < length)
        {
            ByteVector bdata = cmd->read(icLocation->code,
                                         (unsigned short)(icLocation->block + (i / 16)));
            if (bdata.size() == 0)
                break;
            data.insert(data.end(), bdata.begin(), bdata.end());
            i += bdata.size();
        }
    }
    else
    {
        data = cmd->read(icLocation->code, icLocation->block);
    }

    if (data.size() > length)
    {
        data = ByteVector(data.begin(), data.begin() + length);
    }

    return data;
}

ByteVector
    FeliCaStorageCardService::readDataHeader(std::shared_ptr<Location> /*location*/,
                                             std::shared_ptr<AccessInfo> /*aiToUse*/)
{
    return {};
}
}