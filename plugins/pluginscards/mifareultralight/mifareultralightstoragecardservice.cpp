/**
 * \file mifareultralighstoragecardservice.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Mifare Ultralight storage card service.
 */

#include "mifareultralightstoragecardservice.hpp"
#include "mifareultralightchip.hpp"
#include "mifareultralightaccessinfo.hpp"

namespace logicalaccess
{
	MifareUltralightStorageCardService::MifareUltralightStorageCardService(boost::shared_ptr<Chip> chip)
		: StorageCardService(chip)
	{

	}

	MifareUltralightStorageCardService::~MifareUltralightStorageCardService()
	{

	}

	void MifareUltralightStorageCardService::erase(boost::shared_ptr<Location> location, boost::shared_ptr<AccessInfo> aiToUse)
	{
		boost::shared_ptr<MifareUltralightLocation> mLocation = boost::dynamic_pointer_cast<MifareUltralightLocation>(location);
		if (!mLocation)
		{
			return;
		}

		unsigned char zeroblock[4];
		memset(zeroblock, 0x00, sizeof(zeroblock));

		writeData(location, aiToUse, boost::shared_ptr<AccessInfo>(), zeroblock, sizeof(zeroblock), CB_DEFAULT);
	}	

	void MifareUltralightStorageCardService::writeData(boost::shared_ptr<Location> location, boost::shared_ptr<AccessInfo> /*aiToUse*/, boost::shared_ptr<AccessInfo> /*aiToWrite*/, const void* data, size_t dataLength, CardBehavior behaviorFlags)
	{
		EXCEPTION_ASSERT_WITH_LOG(location, std::invalid_argument, "location cannot be null.");
		EXCEPTION_ASSERT_WITH_LOG(data, std::invalid_argument, "data cannot be null.");

		boost::shared_ptr<MifareUltralightLocation> mLocation = boost::dynamic_pointer_cast<MifareUltralightLocation>(location);
		EXCEPTION_ASSERT_WITH_LOG(mLocation, std::invalid_argument, "location must be a MifareUltralightLocation.");
		boost::shared_ptr<MifareUltralightAccessInfo> mAi = boost::dynamic_pointer_cast<MifareUltralightAccessInfo>(location);

		size_t totaldatalen = dataLength + mLocation->byte;
		int nbPages = 0;
		size_t buflen = 0;
		while (buflen < totaldatalen)
		{
			buflen += 4;
			nbPages++;
		}
	
		if (nbPages >= 1)
		{
			std::vector<unsigned char> dataPages;
			dataPages.resize(buflen, 0x00);
			memcpy(&dataPages[0] + mLocation->byte, data, dataLength);

			size_t reallen;			

			if (behaviorFlags & CB_AUTOSWITCHAREA)
			{
				reallen = getMifareUltralightChip()->getMifareUltralightCommands()->writePages(mLocation->page,
					mLocation->page + nbPages - 1,
					&dataPages[0],
					dataPages.size()
				);
			}
			else
			{
				reallen = getMifareUltralightChip()->getMifareUltralightCommands()->writePage(mLocation->page,
					&dataPages[0],
					dataPages.size()
				);
			}

			if ((reallen >= buflen) && mAi)
			{
				if (mAi->lockPage)
				{
					for (int i = mLocation->page; i < mLocation->page + nbPages; ++i)
					{
						getMifareUltralightChip()->getMifareUltralightCommands()->lockPage(i);
					}
				}
			}
		}
	}

	void MifareUltralightStorageCardService::readData(boost::shared_ptr<Location> location, boost::shared_ptr<AccessInfo> /*aiToUse*/, void* data, size_t dataLength, CardBehavior behaviorFlags)
	{
		EXCEPTION_ASSERT_WITH_LOG(location, std::invalid_argument, "location cannot be null.");
		EXCEPTION_ASSERT_WITH_LOG(data, std::invalid_argument, "data cannot be null.");

		boost::shared_ptr<MifareUltralightLocation> mLocation = boost::dynamic_pointer_cast<MifareUltralightLocation>(location);
		EXCEPTION_ASSERT_WITH_LOG(mLocation, std::invalid_argument, "location must be a MifareLocation.");

		size_t totaldatalen = dataLength + mLocation->byte;
		int nbPages = 0;
		size_t buflen = 0;
		while (buflen < totaldatalen)
		{
			buflen += 4;
			nbPages++;
		}

		if (nbPages >= 1)
		{
			std::vector<unsigned char> dataPages;
			dataPages.resize(buflen, 0x00);
			size_t reallen;

			if (behaviorFlags & CB_AUTOSWITCHAREA)
			{
				reallen = getMifareUltralightChip()->getMifareUltralightCommands()->readPages(mLocation->page, mLocation->page + nbPages - 1, dataPages.data(), dataPages.size()); 
			}
			else
			{
				reallen = getMifareUltralightChip()->getMifareUltralightCommands()->readPage(mLocation->page, dataPages.data(), dataPages.size()); 
			}

			if (dataLength <= (reallen - mLocation->byte))
			{
				memcpy(static_cast<char*>(data), &dataPages[0] + mLocation->byte, dataLength);
			}
		}
	}

	unsigned int MifareUltralightStorageCardService::readDataHeader(boost::shared_ptr<Location> /*location*/, boost::shared_ptr<AccessInfo> /*aiToUse*/, void* /*data*/, size_t /*dataLength*/)
	{
		return 0;
	}

	void MifareUltralightStorageCardService::erase()
	{
		unsigned char zeroblock[4];

		memset(zeroblock, 0x00, sizeof(zeroblock));		

		for (unsigned int i = 4; i < 16; ++i)
		{
			getMifareUltralightChip()->getMifareUltralightCommands()->writePage(i, zeroblock, sizeof(zeroblock));
		}
	}
}
