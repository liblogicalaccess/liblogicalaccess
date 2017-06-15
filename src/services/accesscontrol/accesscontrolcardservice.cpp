/**
 * \file accesscontrolcardservice.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Access Control Card service.
 */

#include "logicalaccess/services/accesscontrol/accesscontrolcardservice.hpp"
#include "logicalaccess/services/storage/storagecardservice.hpp"
#include "logicalaccess/cards/chip.hpp"
#include "logicalaccess/services/accesscontrol/formats/wiegand26format.hpp"
#include "logicalaccess/services/accesscontrol/formats/wiegand34format.hpp"
#include "logicalaccess/services/accesscontrol/formats/wiegand34withfacilityformat.hpp"
#include "logicalaccess/services/accesscontrol/formats/wiegand37format.hpp"
#include "logicalaccess/services/accesscontrol/formats/wiegand37withfacilityformat.hpp"
#include "logicalaccess/services/accesscontrol/formats/corporate1000format.hpp"
#include "logicalaccess/services/accesscontrol/formats/dataclockformat.hpp"

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

				formatret->setLinearData(formatBuf);
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

			formatBuf = format->getLinearData();
            storage->writeData(location, aiToUse, aiToWrite, formatBuf, CB_AUTOSWITCHAREA);
        }
		else
			EXCEPTION_ASSERT_WITH_LOG(location, std::runtime_error, "No storage has been found.");
		return true;
    }

	FormatList AccessControlCardService::getHIDWiegandFormatList()
	{
		FormatList formats;

		formats.push_back(std::shared_ptr<Wiegand26Format>(new Wiegand26Format()));
		formats.push_back(std::shared_ptr<Wiegand34Format>(new Wiegand34Format()));
		formats.push_back(std::shared_ptr<Wiegand34WithFacilityFormat>(new Wiegand34WithFacilityFormat()));
		formats.push_back(std::shared_ptr<Wiegand37Format>(new Wiegand37Format()));
		formats.push_back(std::shared_ptr<Wiegand37WithFacilityFormat>(new Wiegand37WithFacilityFormat()));
		formats.push_back(std::shared_ptr<DataClockFormat>(new DataClockFormat()));
		formats.push_back(std::shared_ptr<Corporate1000Format>(new Corporate1000Format()));

		return formats;
	}

	FormatList AccessControlCardService::getSupportedFormatList()
	{
		return getHIDWiegandFormatList();
	}
}