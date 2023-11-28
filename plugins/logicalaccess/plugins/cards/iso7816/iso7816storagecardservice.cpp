/**
 * \file iso7816storagecardservice.cpp
 * \author Maxime C. <maxime@leosac.com>
 * \brief ISO7816 storage card service.
 */

#include <cstring>
#include <logicalaccess/plugins/llacommon/logs.hpp>
#include <logicalaccess/plugins/cards/iso7816/iso7816storagecardservice.hpp>
#include <logicalaccess/services/accesscontrol/formats/bithelper.hpp>
#include <logicalaccess/plugins/cards/iso7816/iso7816location.hpp>
#include <logicalaccess/plugins/cards/iso7816/iso7816chip.hpp>
#include <logicalaccess/myexception.hpp>

namespace logicalaccess
{
ISO7816StorageCardService::ISO7816StorageCardService(std::shared_ptr<Chip> chip)
    : StorageCardService(chip)
{
}

ISO7816StorageCardService::~ISO7816StorageCardService()
{
}

void ISO7816StorageCardService::erase(std::shared_ptr<Location> location,
                                      std::shared_ptr<AccessInfo> /*aiToUse*/)
{
    EXCEPTION_ASSERT_WITH_LOG(location, std::invalid_argument,
                              "location cannot be null.");
    std::shared_ptr<ISO7816Location> icLocation =
        std::dynamic_pointer_cast<ISO7816Location>(location);

    EXCEPTION_ASSERT_WITH_LOG(icLocation, std::invalid_argument,
                              "location must be a ISO7816Location.");

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
            getISO7816Chip()->getISO7816Commands()->selectFile(ByteVector(icLocation->dfname, icLocation->dfname + icLocation->dfnamelen));
        }
    }

    switch (icLocation->fileType)
    {
    case IFT_TRANSPARENT: { getISO7816Chip()->getISO7816Commands()->eraseBinary(0);
    }
    break;

    case IFT_LINEAR_FIXED:
    case IFT_LINEAR_VARIABLE:
    case IFT_CYCLIC:
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Not implemented yet.")

    default:
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "Doesn't know how to write on this file.")
    }
}

void ISO7816StorageCardService::writeData(std::shared_ptr<Location> location,
                                          std::shared_ptr<AccessInfo> /*aiToUse*/,
                                          std::shared_ptr<AccessInfo> /*aiToWrite*/,
                                          const ByteVector &data,
                                          CardBehavior /*behaviorFlags*/)
{
    EXCEPTION_ASSERT_WITH_LOG(location, std::invalid_argument,
                              "location cannot be null.");

    std::shared_ptr<ISO7816Location> icLocation =
        std::dynamic_pointer_cast<ISO7816Location>(location);

    EXCEPTION_ASSERT_WITH_LOG(icLocation, std::invalid_argument,
                              "location must be a ISO7816Location.");

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
            getISO7816Chip()->getISO7816Commands()->selectFile(ByteVector(icLocation->dfname, icLocation->dfname + icLocation->dfnamelen));
        }
    }

    switch (icLocation->fileType)
    {
    case IFT_MASTER:
    case IFT_DIRECTORY:
    {
        if (icLocation->dataObject > 0)
        {
            getISO7816Chip()->getISO7816Commands()->putData(data, icLocation->dataObject);
        }
        else
        {
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                     "Please specify a data object.");
        }
    }
    break;

    case IFT_TRANSPARENT: { getISO7816Chip()->getISO7816Commands()->writeBinary(data, 0);
    }
    break;

    case IFT_LINEAR_FIXED:
    case IFT_LINEAR_VARIABLE:
    case IFT_CYCLIC:
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Not implemented yet.")

    default:
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "Doesn't know how to write on this file.")
    }
}

ByteVector ISO7816StorageCardService::readData(std::shared_ptr<Location> location,
                                               std::shared_ptr<AccessInfo> /*aiToUse*/,
                                               size_t length,
                                               CardBehavior /*behaviorFlags*/)
{
    ByteVector data;
    EXCEPTION_ASSERT_WITH_LOG(location, std::invalid_argument,
                              "location cannot be null.");

    std::shared_ptr<ISO7816Location> icLocation =
        std::dynamic_pointer_cast<ISO7816Location>(location);

    EXCEPTION_ASSERT_WITH_LOG(icLocation, std::invalid_argument,
                              "location must be a ISO7816Location.");

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
            getISO7816Chip()->getISO7816Commands()->selectFile(ByteVector(icLocation->dfname, icLocation->dfname + icLocation->dfnamelen));
        }
    }

    switch (icLocation->fileType)
    {
    case IFT_MASTER:
    case IFT_DIRECTORY:
        if (icLocation->dataObject > 0)
        {
            data = getISO7816Chip()->getISO7816Commands()->getData(
                length, icLocation->dataObject);
        }
        else
        {
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                     "Please specify a data object.");
        }
        break;

    case IFT_TRANSPARENT:
        data = getISO7816Chip()->getISO7816Commands()->readBinary(length, 0);
        break;

    case IFT_LINEAR_FIXED:
    case IFT_LINEAR_VARIABLE:
    case IFT_CYCLIC:
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Not implemented yet.")

    default:
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "Doesn't know how to read on this file.")
    }
    return data;
}

ByteVector
    ISO7816StorageCardService::readDataHeader(std::shared_ptr<Location> /*location*/,
                                              std::shared_ptr<AccessInfo> /*aiToUse*/)
{
    return {};
}
}