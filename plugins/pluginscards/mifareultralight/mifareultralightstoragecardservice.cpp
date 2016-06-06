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

        std::vector<unsigned char> zeroblock(4, 0x00);

        writeData(location, aiToUse, std::shared_ptr<AccessInfo>(), zeroblock, CB_DEFAULT);
    }

    void MifareUltralightStorageCardService::writeData(std::shared_ptr<Location> location, std::shared_ptr<AccessInfo> /*aiToUse*/, std::shared_ptr<AccessInfo> aiToWrite, const std::vector<unsigned char>& data, CardBehavior behaviorFlags)
    {
        EXCEPTION_ASSERT_WITH_LOG(location, std::invalid_argument, "location cannot be null.");

        std::shared_ptr<MifareUltralightLocation> mLocation = std::dynamic_pointer_cast<MifareUltralightLocation>(location);
        EXCEPTION_ASSERT_WITH_LOG(mLocation, std::invalid_argument, "location must be a MifareUltralightLocation.");
        std::shared_ptr<MifareUltralightAccessInfo> mAi = std::dynamic_pointer_cast<MifareUltralightAccessInfo>(aiToWrite);

        size_t totaldatalen = data.size() + mLocation->byte;
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
			std::copy(data.begin(), data.end(), dataPages.begin() + mLocation->byte);

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

    std::vector<unsigned char> MifareUltralightStorageCardService::readData(std::shared_ptr<Location> location, std::shared_ptr<AccessInfo> /*aiToUse*/, size_t length, CardBehavior behaviorFlags)
    {
        EXCEPTION_ASSERT_WITH_LOG(location, std::invalid_argument, "location cannot be null.");
		std::vector<unsigned char> ret;
        std::shared_ptr<MifareUltralightLocation> mLocation = std::dynamic_pointer_cast<MifareUltralightLocation>(location);
        EXCEPTION_ASSERT_WITH_LOG(mLocation, std::invalid_argument, "location must be a MifareLocation.");

        size_t totaldatalen = length + mLocation->byte;
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

            if (behaviorFlags & CB_AUTOSWITCHAREA)
            {
                dataPages = getMifareUltralightChip()->getMifareUltralightCommands()->readPages(mLocation->page, mLocation->page + nbPages - 1);
            }
            else
            {
                dataPages = getMifareUltralightChip()->getMifareUltralightCommands()->readPage(mLocation->page);
            }

			ret.insert(ret.end(), dataPages.begin() + mLocation->byte, dataPages.begin() + mLocation->byte + length);
        }
		return ret;
    }

    unsigned int MifareUltralightStorageCardService::readDataHeader(std::shared_ptr<Location> /*location*/, std::shared_ptr<AccessInfo> /*aiToUse*/, void* /*data*/, size_t /*dataLength*/)
    {
        return 0;
    }

    void MifareUltralightStorageCardService::erase()
    {
		std::vector<unsigned char> zeroblock(4, 0x00);
        for (unsigned int i = 4; i < getMifareUltralightChip()->getNbBlocks(); ++i)
        {
            getMifareUltralightChip()->getMifareUltralightCommands()->writePage(i, zeroblock);
        }
    }
}