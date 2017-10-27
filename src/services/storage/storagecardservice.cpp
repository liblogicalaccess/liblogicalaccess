/**
 * \file storagecardservice.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Storage Card service.
 */

#include "logicalaccess/services/storage/storagecardservice.hpp"

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