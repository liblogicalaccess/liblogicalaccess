/**
 * \file storagecardservice.cpp
 * \author Maxime C. <maxime@leosac.com>
 * \brief Storage Card service.
 */

#include <logicalaccess/services/storage/storagecardservice.hpp>

namespace logicalaccess
{
StorageCardService::StorageCardService(std::shared_ptr<Chip> chip)
    : CardService(chip, CST_STORAGE)
{
}

StorageCardService::~StorageCardService()
{
}
}