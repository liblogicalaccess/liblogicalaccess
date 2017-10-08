/**
 * \file mifareultralighstoragecardservice.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Mifare Ultralight storage card service.
 */

#include <logicalaccess/logs.hpp>
#include "mifareultralightstoragecardservice.hpp"
#include "mifareultralightchip.hpp"
#include "mifareultralightaccessinfo.hpp"

namespace logicalaccess
{
    MifareUltralightStorageCardService::MifareUltralightStorageCardService(std::shared_ptr<Chip> chip)
        : StorageCardService(chip)
    {
    }

    MifareUltralightStorageCardService::~MifareUltralightStorageCardService()
    {
    }

    void MifareUltralightStorageCardService::erase(std::shared_ptr<Location> location, std::shared_ptr<AccessInfo> aiToUse)
    {
        std::shared_ptr<MifareUltralightLocation> mLocation = std::dynamic_pointer_cast<MifareUltralightLocation>(location);
        if (!mLocation)
        {
            return;
        }

        ByteVector zeroblock(4, 0x00);

        writeData(location, aiToUse, std::shared_ptr<AccessInfo>(), zeroblock, CB_DEFAULT);
    }

    void MifareUltralightStorageCardService::writeData(std::shared_ptr<Location> location, std::shared_ptr<AccessInfo> /*aiToUse*/, std::shared_ptr<AccessInfo> aiToWrite, const ByteVector& data, CardBehavior behaviorFlags)
    {
        EXCEPTION_ASSERT_WITH_LOG(location, std::invalid_argument, "location cannot be null.");

        std::shared_ptr<MifareUltralightLocation> mLocation = std::dynamic_pointer_cast<MifareUltralightLocation>(location);
        EXCEPTION_ASSERT_WITH_LOG(mLocation, std::invalid_argument, "location must be a MifareUltralightLocation.");
        std::shared_ptr<MifareUltralightAccessInfo> mAi = std::dynamic_pointer_cast<MifareUltralightAccessInfo>(aiToWrite);

        size_t totaldatalen = data.size() + mLocation->byte_;
        int nbPages = 0;
        size_t buflen = 0;
        while (buflen < totaldatalen)
        {
            buflen += 4;
            nbPages++;
        }

        if (nbPages >= 1)
        {
            ByteVector dataPages;
            dataPages.resize(buflen, 0x00);
			copy(data.begin(), data.end(), dataPages.begin() + mLocation->byte_);

            if (behaviorFlags & CB_AUTOSWITCHAREA)
            {
                getMifareUltralightChip()->getMifareUltralightCommands()->writePages(mLocation->page,
                    mLocation->page + nbPages - 1, dataPages);
            }
            else
            {
				getMifareUltralightChip()->getMifareUltralightCommands()->writePage(mLocation->page, dataPages);
            }

            if (mAi && mAi->lockPage)
            {
                for (int i = mLocation->page; i < mLocation->page + nbPages; ++i)
                {
                    getMifareUltralightChip()->getMifareUltralightCommands()->lockPage(i);
                }
            }
        }
    }

    ByteVector MifareUltralightStorageCardService::readData(std::shared_ptr<Location> location, std::shared_ptr<AccessInfo> /*aiToUse*/, size_t length, CardBehavior behaviorFlags)
    {
        EXCEPTION_ASSERT_WITH_LOG(location, std::invalid_argument, "location cannot be null.");
		ByteVector ret;
        std::shared_ptr<MifareUltralightLocation> mLocation = std::dynamic_pointer_cast<MifareUltralightLocation>(location);
        EXCEPTION_ASSERT_WITH_LOG(mLocation, std::invalid_argument, "location must be a MifareLocation.");

        size_t totaldatalen = length + mLocation->byte_;
        int nbPages = 0;
        size_t buflen = 0;
        while (buflen < totaldatalen)
        {
            buflen += 4;
            nbPages++;
        }

        if (nbPages >= 1)
        {
            ByteVector dataPages;

            if (behaviorFlags & CB_AUTOSWITCHAREA)
            {
                dataPages = getMifareUltralightChip()->getMifareUltralightCommands()->readPages(mLocation->page, mLocation->page + nbPages - 1);
            }
            else
            {
                dataPages = getMifareUltralightChip()->getMifareUltralightCommands()->readPage(mLocation->page);
            }

            // Some commands implementation returns more than one block (eg. PC/SC)
            if (dataPages.size() > 4)
            {
                nbPages += static_cast<int>((dataPages.size() + 3) / 4) - 1;
            }

			ret.insert(ret.end(), dataPages.begin() + mLocation->byte_, dataPages.begin() + mLocation->byte_ + length);
        }
		return ret;
    }

	ByteVector MifareUltralightStorageCardService::readDataHeader(std::shared_ptr<Location> /*location*/, std::shared_ptr<AccessInfo> /*aiToUse*/)
    {
        return {};
    }
}