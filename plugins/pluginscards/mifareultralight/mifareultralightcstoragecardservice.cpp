/**
 * \file mifareultralightcstoragecardservice.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Mifare Ultralight C storage card service.
 */

#include "mifareultralightcstoragecardservice.hpp"
#include "mifareultralightcchip.hpp"

namespace logicalaccess
{
	MifareUltralightCStorageCardService::MifareUltralightCStorageCardService(boost::shared_ptr<Chip> chip)
		: MifareUltralightStorageCardService(chip)
	{

	}

	MifareUltralightCStorageCardService::~MifareUltralightCStorageCardService()
	{

	}

	bool MifareUltralightCStorageCardService::erase()
	{
		getMifareUltralightCChip()->getMifareUltralightCCommands()->authenticate(getMifareUltralightCChip()->getMifareUltralightCProfile()->getKey());

		return MifareUltralightStorageCardService::erase();
	}

	bool MifareUltralightCStorageCardService::erase(boost::shared_ptr<Location> location, boost::shared_ptr<AccessInfo> aiToUse)
	{
		getMifareUltralightCChip()->getMifareUltralightCCommands()->authenticate(aiToUse);

		return MifareUltralightStorageCardService::erase(location, aiToUse);
	}

	bool MifareUltralightCStorageCardService::writeData(boost::shared_ptr<Location> location, boost::shared_ptr<AccessInfo> aiToUse, boost::shared_ptr<AccessInfo> aiToWrite, const void* data, size_t dataLength, CardBehavior behaviorFlags)
	{
		getMifareUltralightCChip()->getMifareUltralightCCommands()->authenticate(aiToUse);

		return MifareUltralightStorageCardService::writeData(location, aiToUse, aiToWrite, data, dataLength, behaviorFlags);
	}

	bool MifareUltralightCStorageCardService::readData(boost::shared_ptr<Location> location, boost::shared_ptr<AccessInfo> aiToUse, void* data, size_t dataLength, CardBehavior behaviorFlags)
	{
		getMifareUltralightCChip()->getMifareUltralightCCommands()->authenticate(aiToUse);

		return MifareUltralightStorageCardService::readData(location, aiToUse, data, dataLength, behaviorFlags);
	}
}