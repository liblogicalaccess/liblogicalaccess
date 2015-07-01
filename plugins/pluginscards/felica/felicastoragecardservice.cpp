/**
 * \file felicastoragecardservice.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief FeliCa storage card service.
 */

#include <logicalaccess/logs.hpp>
#include "felicastoragecardservice.hpp"
#include "felicalocation.hpp"
#include "logicalaccess/cards/locationnode.hpp"

namespace logicalaccess
{
	FeliCaStorageCardService::FeliCaStorageCardService(std::shared_ptr<Chip> chip)
        : StorageCardService(chip)
    {
    }

	FeliCaStorageCardService::~FeliCaStorageCardService()
    {
    }

	void FeliCaStorageCardService::erase()
    {
        // Not supported
    }

	void FeliCaStorageCardService::erase(std::shared_ptr<Location> location, std::shared_ptr<AccessInfo> aiToUse)
    {
		// Not supported
    }

	void FeliCaStorageCardService::writeData(std::shared_ptr<Location> location, std::shared_ptr<AccessInfo>, std::shared_ptr<AccessInfo>, const std::vector<unsigned char>& data, CardBehavior)
    {
        EXCEPTION_ASSERT_WITH_LOG(location, std::invalid_argument, "location cannot be null.");

		std::shared_ptr<FeliCaLocation> icLocation = std::dynamic_pointer_cast<FeliCaLocation>(location);

        EXCEPTION_ASSERT_WITH_LOG(icLocation, std::invalid_argument, "location must be a FeliCaLocation.");

		getFeliCaChip()->getFeliCaCommands()->write(icLocation->code, icLocation->block, data);
    }

	std::vector<unsigned char> FeliCaStorageCardService::readData(std::shared_ptr<Location> location, std::shared_ptr<AccessInfo>, size_t length, CardBehavior)
    {
        EXCEPTION_ASSERT_WITH_LOG(location, std::invalid_argument, "location cannot be null.");

		std::shared_ptr<FeliCaLocation> icLocation = std::dynamic_pointer_cast<FeliCaLocation>(location);

        EXCEPTION_ASSERT_WITH_LOG(icLocation, std::invalid_argument, "location must be a FeliCaLocation.");

		return getFeliCaChip()->getFeliCaCommands()->read(icLocation->code, icLocation->block);
    }

	unsigned int FeliCaStorageCardService::readDataHeader(std::shared_ptr<Location>, std::shared_ptr<AccessInfo>, void*, size_t)
    {
        return 0;
    }
}