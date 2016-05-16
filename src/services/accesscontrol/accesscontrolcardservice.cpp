/**
 * \file accesscontrolcardservice.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Access Control Card service.
 */

#include "logicalaccess/services/accesscontrol/accesscontrolcardservice.hpp"
#include "logicalaccess/services/storage/storagecardservice.hpp"
#include "logicalaccess/cards/chip.hpp"

namespace logicalaccess
{
    AccessControlCardService::AccessControlCardService(std::shared_ptr<Chip> chip)
		: CardService(chip, CST_ACCESS_CONTROL)
    {
    }

    AccessControlCardService::~AccessControlCardService()
    {
    }

    std::shared_ptr<Format> AccessControlCardService::readFormat(std::shared_ptr<Format> format, std::shared_ptr<Location> location, std::shared_ptr<AccessInfo> aiToUse)
    {
        bool ret = false;

        EXCEPTION_ASSERT_WITH_LOG(format, std::invalid_argument, "format to read can't be null.");
        EXCEPTION_ASSERT_WITH_LOG(location, std::invalid_argument, "location parameter can't be null.");

        // By default duplicate the format. Other kind of implementation should override this current method.
        std::shared_ptr<Format> formatret = Format::getByFormatType(format->getType());
        formatret->unSerialize(format->serialize(), "");

        std::shared_ptr<StorageCardService> storage = std::dynamic_pointer_cast<StorageCardService>(d_chip->getService(CST_STORAGE));
        if (storage)
        {
            size_t length = (formatret->getDataLength() + 7) / 8;
            if (length > 0)
            {
				std::vector<unsigned char> formatBuf;

				formatBuf = storage->readData(location, aiToUse, length, CB_AUTOSWITCHAREA);

				formatret->setLinearData(&formatBuf[0], formatBuf.size());
				ret = true;
            }
        }

        if (!ret)
            formatret.reset();

        return formatret;
    }

    bool AccessControlCardService::writeFormat(std::shared_ptr<Format> format, std::shared_ptr<Location> location, std::shared_ptr<AccessInfo> aiToUse, std::shared_ptr<AccessInfo> aiToWrite)
    {
        EXCEPTION_ASSERT_WITH_LOG(format, std::invalid_argument, "format to write can't be null.");
        EXCEPTION_ASSERT_WITH_LOG(location, std::invalid_argument, "location parameter can't be null.");

        std::shared_ptr<StorageCardService> storage = std::dynamic_pointer_cast<StorageCardService>(d_chip->getService(CST_STORAGE));
        if (storage)
        {
            size_t length = (format->getDataLength() + 7) / 8;
			std::vector<unsigned char> formatBuf(length, 0x00);

            format->getLinearData(&formatBuf[0], formatBuf.size());
            storage->writeData(location, aiToUse, aiToWrite, formatBuf, CB_AUTOSWITCHAREA);
        }
		else
			EXCEPTION_ASSERT_WITH_LOG(location, std::runtime_error, "No storage has been found.");
		return true;
    }
}