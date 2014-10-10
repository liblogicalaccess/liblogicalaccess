/**
 * \file iso7816storagecardservice.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief ISO7816 storage card service.
 */

#include "iso7816storagecardservice.hpp"
#include "logicalaccess/services/accesscontrol/formats/bithelper.hpp"
#include "iso7816location.hpp"
#include "iso7816chip.hpp"

namespace logicalaccess
{
    ISO7816StorageCardService::ISO7816StorageCardService(boost::shared_ptr<Chip> chip)
        : StorageCardService(chip)
    {
    }

    ISO7816StorageCardService::~ISO7816StorageCardService()
    {
    }

    void ISO7816StorageCardService::erase()
    {
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Not implemented.");
    }

    void ISO7816StorageCardService::erase(boost::shared_ptr<Location> location, boost::shared_ptr<AccessInfo> /*aiToUse*/)
    {
        EXCEPTION_ASSERT_WITH_LOG(location, std::invalid_argument, "location cannot be null.");
        boost::shared_ptr<ISO7816Location> icLocation = boost::dynamic_pointer_cast<ISO7816Location>(location);

        EXCEPTION_ASSERT_WITH_LOG(icLocation, std::invalid_argument, "location must be a ISO7816Location.");

        if (icLocation->fileid != 0)
        {
            getISO7816Chip()->getISO7816Commands()->selectFile(icLocation->fileid);
        }
        else
        {
            unsigned char defaultdf[16];
            memset(defaultdf, 0x00, sizeof(defaultdf));
            if (memcmp(icLocation->dfname, defaultdf, sizeof(defaultdf)))
            {
                getISO7816Chip()->getISO7816Commands()->selectFile(icLocation->dfname, icLocation->dfnamelen);
            }
        }

        switch (icLocation->fileType)
        {
        case IFT_TRANSPARENT:
        {
            getISO7816Chip()->getISO7816Commands()->eraseBinary(0);
        }
            break;

        case IFT_LINEAR_FIXED:
        case IFT_LINEAR_VARIABLE:
        case IFT_CYCLIC:
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Not implemented yet.");
            break;

        default:
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Doesn't know how to write on this file.");
            break;
        }
    }

    void ISO7816StorageCardService::writeData(boost::shared_ptr<Location> location, boost::shared_ptr<AccessInfo> /*aiToUse*/, boost::shared_ptr<AccessInfo> /*aiToWrite*/, const void* data, size_t dataLength, CardBehavior /*behaviorFlags*/)
    {
        EXCEPTION_ASSERT_WITH_LOG(location, std::invalid_argument, "location cannot be null.");
        EXCEPTION_ASSERT_WITH_LOG(data, std::invalid_argument, "data cannot be null.");

        boost::shared_ptr<ISO7816Location> icLocation = boost::dynamic_pointer_cast<ISO7816Location>(location);

        EXCEPTION_ASSERT_WITH_LOG(icLocation, std::invalid_argument, "location must be a ISO7816Location.");

        if (icLocation->fileid != 0)
        {
            getISO7816Chip()->getISO7816Commands()->selectFile(icLocation->fileid);
        }
        else
        {
            unsigned char defaultdf[16];
            memset(defaultdf, 0x00, sizeof(defaultdf));
            if (memcmp(icLocation->dfname, defaultdf, sizeof(defaultdf)))
            {
                getISO7816Chip()->getISO7816Commands()->selectFile(icLocation->dfname, icLocation->dfnamelen);
            }
        }

        switch (icLocation->fileType)
        {
        case IFT_MASTER:
        case IFT_DIRECTORY:
        {
            if (icLocation->dataObject > 0)
            {
                getISO7816Chip()->getISO7816Commands()->putData(data, dataLength, icLocation->dataObject);
            }
            else
            {
                THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Please specify a data object.");
            }
        }
            break;

        case IFT_TRANSPARENT:
        {
            getISO7816Chip()->getISO7816Commands()->writeBinary(data, dataLength, 0);
        }
            break;

        case IFT_LINEAR_FIXED:
        case IFT_LINEAR_VARIABLE:
        case IFT_CYCLIC:
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Not implemented yet.");
            break;

        default:
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Doesn't know how to write on this file.");
            break;
        }
    }

    void ISO7816StorageCardService::readData(boost::shared_ptr<Location> location, boost::shared_ptr<AccessInfo> /*aiToUse*/, void* data, size_t dataLength, CardBehavior /*behaviorFlags*/)
    {
        EXCEPTION_ASSERT_WITH_LOG(location, std::invalid_argument, "location cannot be null.");
        EXCEPTION_ASSERT_WITH_LOG(data, std::invalid_argument, "data cannot be null.");

        boost::shared_ptr<ISO7816Location> icLocation = boost::dynamic_pointer_cast<ISO7816Location>(location);

        EXCEPTION_ASSERT_WITH_LOG(icLocation, std::invalid_argument, "location must be a ISO7816Location.");

        if (icLocation->fileid != 0)
        {
            getISO7816Chip()->getISO7816Commands()->selectFile(icLocation->fileid);
        }
        else
        {
            unsigned char defaultdf[16];
            memset(defaultdf, 0x00, sizeof(defaultdf));
            if (memcmp(icLocation->dfname, defaultdf, sizeof(defaultdf)))
            {
                getISO7816Chip()->getISO7816Commands()->selectFile(icLocation->dfname, icLocation->dfnamelen);
            }
        }

        switch (icLocation->fileType)
        {
        case IFT_MASTER:
        case IFT_DIRECTORY:
            if (icLocation->dataObject > 0)
            {
                getISO7816Chip()->getISO7816Commands()->getData(data, dataLength, icLocation->dataObject);
            }
            else
            {
                THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Please specify a data object.");
            }
            break;

        case IFT_TRANSPARENT:
            getISO7816Chip()->getISO7816Commands()->readBinary(data, dataLength, 0);
            break;

        case IFT_LINEAR_FIXED:
        case IFT_LINEAR_VARIABLE:
        case IFT_CYCLIC:
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Not implemented yet.");
            break;

        default:
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Doesn't know how to read on this file.");
            break;
        }
    }

    unsigned int ISO7816StorageCardService::readDataHeader(boost::shared_ptr<Location> /*location*/, boost::shared_ptr<AccessInfo> /*aiToUse*/, void* /*data*/, size_t /*dataLength*/)
    {
        return 0;
    }
}