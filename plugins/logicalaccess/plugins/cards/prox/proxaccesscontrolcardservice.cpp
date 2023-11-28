/**
 * \file proxaccesscontrolcardservice.cpp
 * \author Maxime C. <maxime@leosac.com>
 * \brief Prox Access Control Card service.
 */

#include <logicalaccess/plugins/cards/prox/proxaccesscontrolcardservice.hpp>
#include <logicalaccess/cards/readercardadapter.hpp>
#include <logicalaccess/plugins/cards/prox/proxchip.hpp>
#include <logicalaccess/plugins/cards/prox/proxlocation.hpp>
#include <logicalaccess/services/accesscontrol/formats/bithelper.hpp>
#include <logicalaccess/services/accesscontrol/formats/rawformat.hpp>

#if defined(__unix__)
#include <cstring>
#endif

namespace logicalaccess
{
ProxAccessControlCardService::ProxAccessControlCardService(std::shared_ptr<Chip> chip)
    : AccessControlCardService(chip)
{
}

ProxAccessControlCardService::~ProxAccessControlCardService()
{
}

std::shared_ptr<Format>
ProxAccessControlCardService::readFormat(std::shared_ptr<Format> format,
                                         std::shared_ptr<Location> location,
                                         std::shared_ptr<AccessInfo> /*aiToUse*/)
{
    bool ret = false;

    std::shared_ptr<Format> formatret;
    if (format)
    {
        std::shared_ptr<ProxLocation> pLocation;
        formatret = Format::getByFormatType(format->getType());
        formatret->unSerialize(format->serialize(), "");
        unsigned int dataLengthBits = formatret->getDataLength();
        unsigned int atrLengthBits =
            static_cast<unsigned int>(getChip()->getChipIdentifier().size() * 8);
        if (dataLengthBits == 0)
        {
            dataLengthBits = atrLengthBits;
        }

        if (location)
        {
            pLocation = std::dynamic_pointer_cast<ProxLocation>(location);
        }
        else
        {
            pLocation.reset(new ProxLocation());
            // TODO: fix it (signed/unsigned)
            pLocation->bit =
                static_cast<int>(atrLengthBits - dataLengthBits); // raw wiegand

            int historBytesSize = getChip()->getChipIdentifier()[1] & 0x0f;
            if ((getChip()->getChipIdentifier().size() - historBytesSize - 1) > 0 &&
                getChip()->getChipIdentifier()[getChip()->getChipIdentifier().size() -
                                               historBytesSize - 1] ==
                    0x00) // Check Prox Format
            {
                pLocation->bit -= 8; // skip TCK
                pLocation->bit = static_cast<unsigned char>(pLocation->bit / 8) * 8;
            }
        }

        if (pLocation)
        {
            if (dataLengthBits > 0)
            {
                BitsetStream formatBuf;
                try
                {
                    formatBuf.concat(getChip()->getChipIdentifier(),
                                     static_cast<unsigned int>(pLocation->bit),
                                     dataLengthBits);
                    formatret->setLinearData(formatBuf.getData());
                    ret = true;
                }
                catch (std::exception &)
                {
                    throw;
                }
            }
        }
    }

    if (!ret)
    {
        formatret.reset();
    }

    return formatret;
}

bool ProxAccessControlCardService::writeFormat(std::shared_ptr<Format> /*format*/,
                                               std::shared_ptr<Location> /*location*/,
                                               std::shared_ptr<AccessInfo> /*aiToUse*/,
                                               std::shared_ptr<AccessInfo> /*aiToWrite*/)
{
    return false;
}

ByteVector ProxAccessControlCardService::getPACSBits() const
{
    return getChip()->getChipIdentifier();
}

FormatList ProxAccessControlCardService::getSupportedFormatList()
{
    FormatList list = getHIDWiegandFormatList();
    list.push_back(std::make_shared<RawFormat>());
    return list;
}
}