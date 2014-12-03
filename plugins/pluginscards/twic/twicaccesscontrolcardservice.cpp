/**
 * \file twicaccesscontrolcardservice.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Twic Access Control Card service.
 */

#include "twicaccesscontrolcardservice.hpp"
#include "twicchip.hpp"
#include "twiclocation.hpp"
#include "logicalaccess/cards/readercardadapter.hpp"
#include "logicalaccess/services/accesscontrol/formats/bithelper.hpp"
#include "logicalaccess/services/accesscontrol/formats/fascn200bitformat.hpp"
#include "logicalaccess/services/storage/storagecardservice.hpp"

#if defined(__unix__)
#include <cstring>
#endif

namespace logicalaccess
{
    TwicAccessControlCardService::TwicAccessControlCardService(std::shared_ptr<Chip> chip)
        : AccessControlCardService(chip)
    {
    }

    TwicAccessControlCardService::~TwicAccessControlCardService()
    {
    }

    std::shared_ptr<Format> TwicAccessControlCardService::readFormat(std::shared_ptr<Format> format, std::shared_ptr<Location> location, std::shared_ptr<AccessInfo> /*aiToUse*/)
    {
        bool ret = false;

        std::shared_ptr<Format> formatret;
        if (format)
        {
            formatret = Format::getByFormatType(format->getType());
            formatret->unSerialize(format->serialize(), "");
        }
        else
        {
            formatret.reset(new FASCN200BitFormat());
        }

        std::shared_ptr<TwicLocation> pLocation;
        if (location)
        {
            pLocation = std::dynamic_pointer_cast<TwicLocation>(location);
        }
        else
        {
            if (std::dynamic_pointer_cast<FASCN200BitFormat>(formatret))
            {
                pLocation.reset(new TwicLocation());
                pLocation->dataObject = 0x5FC104;
                pLocation->tag = 0x30;
            }
        }

        if (pLocation)
        {
            size_t length = (formatret->getDataLength() + 7) / 8;
			std::vector<unsigned char> formatBuf;

            std::shared_ptr<StorageCardService> storage = std::dynamic_pointer_cast<StorageCardService>(getTwicChip()->getService(CST_STORAGE));
            if (storage)
            {
                formatBuf = storage->readData(pLocation, std::shared_ptr<AccessInfo>(), length, CB_DEFAULT);
                formatret->setLinearData(&formatBuf[0], formatBuf.size());

                ret = true;
            }
        }

        if (!ret)
        {
            formatret.reset();
        }

        return formatret;
    }

    bool TwicAccessControlCardService::writeFormat(std::shared_ptr<Format> /*format*/, std::shared_ptr<Location> /*location*/, std::shared_ptr<AccessInfo> /*aiToUse*/, std::shared_ptr<AccessInfo> /*aiToWrite*/)
    {
        return false;
    }
}