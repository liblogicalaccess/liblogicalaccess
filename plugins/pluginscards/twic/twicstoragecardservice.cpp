/**
 * \file twicstoragecardservice.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Twic storage card service.
 */

#include "twicstoragecardservice.hpp"


namespace logicalaccess
{
	TwicStorageCardService::TwicStorageCardService(boost::shared_ptr<Chip> chip)
		: ISO7816StorageCardService(chip)
	{
		
	}

	TwicStorageCardService::~TwicStorageCardService()
	{
		
	}

	bool TwicStorageCardService::readData(boost::shared_ptr<Location> location, boost::shared_ptr<AccessInfo> aiToUse, void* data, size_t dataLength, CardBehavior behaviorFlags)
	{
		bool ret = false;

		EXCEPTION_ASSERT_WITH_LOG(location, std::invalid_argument, "location cannot be null.");
		EXCEPTION_ASSERT_WITH_LOG(data, std::invalid_argument, "data cannot be null.");

		boost::shared_ptr<ISO7816Location> icISOLocation = boost::dynamic_pointer_cast<ISO7816Location>(location);
		boost::shared_ptr<TwicLocation> icLocation = boost::dynamic_pointer_cast<TwicLocation>(location);

		if (icISOLocation)
		{
			return ISO7816StorageCardService::readData(location, aiToUse, data, dataLength, behaviorFlags);
		}
		EXCEPTION_ASSERT_WITH_LOG(icLocation, std::invalid_argument, "location must be a TwicLocation or ISO7816Location.");

		getTwicChip()->getTwicCommands()->selectTWICApplication();
		if (icLocation->tag == 0x00)
		{
			size_t dataObjectLength = dataLength;			
			if (getTwicChip()->getTwicCommands()->getTWICData(data, dataObjectLength, icLocation->dataObject) && dataObjectLength == dataLength)
			{
				ret = true;
			}
		}
		else
		{
			// A tag is specified, the user want to get only the tag's data.
			size_t dataObjectLength = getTwicChip()->getTwicCommands()->getDataObjectLength(icLocation->dataObject, true);
			unsigned char* fulldata = new unsigned char[dataObjectLength];

			if (getTwicChip()->getTwicCommands()->getTWICData(fulldata, dataObjectLength, icLocation->dataObject))
			{
				size_t offset = getTwicChip()->getTwicCommands()->getMinimumBytesRepresentation(getTwicChip()->getTwicCommands()->getMaximumDataObjectLength(icLocation->dataObject)) + 1;
				if (offset < dataObjectLength)
				{
					if (getTwicChip()->getTwicCommands()->getTagData(icLocation, &fulldata[offset], dataObjectLength - offset, data, dataLength))
					{
						ret = true;
					}
				}
			}

			delete[] fulldata;
		}

		return ret;
	}
}
