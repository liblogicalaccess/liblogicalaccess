/**
 * \file generictagaccesscontrolcardservice.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief GenericTag Access Control Card service.
 */

#include "generictagaccesscontrolcardservice.hpp"
#include "generictagchip.hpp"
#include "logicalaccess/cards/readercardadapter.hpp"
#include "logicalaccess/services/accesscontrol/formats/bithelper.hpp"

#if defined(__unix__)
#include <cstring>
#endif

namespace logicalaccess
{
GenericTagAccessControlCardService::GenericTagAccessControlCardService(
    std::shared_ptr<Chip> chip)
    : AccessControlCardService(chip)
{
}

GenericTagAccessControlCardService::~GenericTagAccessControlCardService()
{
}

std::shared_ptr<Format>
GenericTagAccessControlCardService::readFormat(std::shared_ptr<Format> format,
                                               std::shared_ptr<Location> /*location*/,
                                               std::shared_ptr<AccessInfo> /*aiToUse*/)
{
    bool ret = false;

    LOG(LogLevel::INFOS) << "Try to read a format from Generic Tag...";

    std::shared_ptr<Format> formatret;
    if (format)
    {
        try
        {
            formatret = Format::getByFormatType(format->getType());
            formatret->unSerialize(format->serialize(), "");
            unsigned int dataLengthBits =
                static_cast<unsigned int>(getChip()->getChipIdentifier().size()) * 8;

            if (dataLengthBits > 0)
            {
                unsigned int length = (dataLengthBits + 7) / 8;
                BitsetStream formatBuf;

                try
                {
                    LOG(LogLevel::INFOS) << "Reading data from Generic Tag...";
                    ByteVector identifier = getChip()->getChipIdentifier();
                    if (length <= identifier.size())
                    {
                        unsigned int realDataLengthBits =
                            getGenericTagChip()->getTagIdBitsLength();
                        if (realDataLengthBits == 0)
                        {
                            realDataLengthBits = length * 8;
                        }

                        if (realDataLengthBits >= formatret->getDataLength())
                        {
                            LOG(LogLevel::INFOS) << "Converting data to format...";
                            formatBuf.concat(identifier,
                                             dataLengthBits - realDataLengthBits,
                                             realDataLengthBits);
                            formatret->setLinearData(formatBuf.getData());
                            ret = true;
                        }
                        else
                        {
                            LOG(LogLevel::ERRORS)
                                << "Cannot read the format: format length ("
                                << formatret->getDataLength()
                                << ") bigger than the total available bits ("
                                << realDataLengthBits << ").";
                        }
                    }
                }
                catch (std::exception &)
                {
                    throw;
                }
            }
        }
        catch (std::exception &ex)
        {
            LOG(LogLevel::ERRORS) << "Error on read format: " << ex.what();
            std::rethrow_exception(std::current_exception());
        }
    }

    if (!ret)
    {
        formatret.reset();
    }

    return formatret;
}

bool GenericTagAccessControlCardService::writeFormat(
    std::shared_ptr<Format> /*format*/, std::shared_ptr<Location> /*location*/,
    std::shared_ptr<AccessInfo> /*aiToUse*/, std::shared_ptr<AccessInfo> /*aiToWrite*/)
{
    return false;
}
}