/**
 * \file cps3storagecardservice.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief CPS3 storage card service.
 */

#include <logicalaccess/logs.hpp>
#include "cps3storagecardservice.hpp"
#include "cps3location.hpp"
#include "logicalaccess/cards/locationnode.hpp"

namespace logicalaccess
{
	CPS3StorageCardService::CPS3StorageCardService(std::shared_ptr<Chip> chip)
        : ISO7816StorageCardService(chip)
    {
    }

	CPS3StorageCardService::~CPS3StorageCardService()
    {
    }

	ByteVector CPS3StorageCardService::readData(std::shared_ptr<Location> location, std::shared_ptr<AccessInfo>, size_t /*length*/, CardBehavior)
    {
        EXCEPTION_ASSERT_WITH_LOG(location, std::invalid_argument, "location cannot be null.");

		std::shared_ptr<CPS3Location> icLocation = std::dynamic_pointer_cast<CPS3Location>(location);

        EXCEPTION_ASSERT_WITH_LOG(icLocation, std::invalid_argument, "location must be a CPS3Location.");

		return ByteVector();
    }
}