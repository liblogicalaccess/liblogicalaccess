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
        : CardService(chip)
    {
    }

    AccessControlCardService::~AccessControlCardService()
    {
    }

    CardServiceType AccessControlCardService::getServiceType() const
    {
        return CST_ACCESS_CONTROL;
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
                unsigned char *formatBuf = new unsigned char[length];
                if (formatBuf != NULL)
                {
                    memset(formatBuf, 0x00, length);
                    try
                    {
                        storage->readData(location, aiToUse, formatBuf, length, CB_AUTOSWITCHAREA);

                        formatret->setLinearData(formatBuf, length);
                        ret = true;
                    }
                    catch (std::exception&)
                    {
                        delete[] formatBuf;
                        throw;
                    }
                    delete[] formatBuf;
                }
            }
        }

        if (!ret)
        {
            formatret.reset();
        }

        return formatret;
    }

    bool AccessControlCardService::writeFormat(std::shared_ptr<Format> format, std::shared_ptr<Location> location, std::shared_ptr<AccessInfo> aiToUse, std::shared_ptr<AccessInfo> aiToWrite)
    {
        bool ret = false;

        EXCEPTION_ASSERT_WITH_LOG(format, std::invalid_argument, "format to write can't be null.");
        EXCEPTION_ASSERT_WITH_LOG(location, std::invalid_argument, "location parameter can't be null.");

        std::shared_ptr<StorageCardService> storage = std::dynamic_pointer_cast<StorageCardService>(d_chip->getService(CST_STORAGE));
        if (storage)
        {
            size_t length = (format->getDataLength() + 7) / 8;
            unsigned char *formatBuf = new unsigned char[length];
            memset(formatBuf, 0x00, length);
            format->getLinearData(formatBuf, length);
            try
            {
                storage->writeData(location, aiToUse, aiToWrite, formatBuf, length, CB_AUTOSWITCHAREA);
            }
            catch (std::exception&)
            {
                delete[] formatBuf;
                throw;
            }

            ret = true;
            delete[] formatBuf;
        }

        return ret;
    }
}