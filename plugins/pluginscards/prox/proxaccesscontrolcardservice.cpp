/**
 * \file proxaccesscontrolcardservice.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Prox Access Control Card service.
 */

#include "proxaccesscontrolcardservice.hpp"
#include "logicalaccess/cards/readercardadapter.hpp"
#include "proxchip.hpp"
#include "proxlocation.hpp"
#include "logicalaccess/services/accesscontrol/formats/bithelper.hpp"
#include "logicalaccess/services/accesscontrol/formats/rawformat.hpp"

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

    std::shared_ptr<Format> ProxAccessControlCardService::readFormat(std::shared_ptr<Format> format, std::shared_ptr<Location> location, std::shared_ptr<AccessInfo> /*aiToUse*/)
    {
        bool ret = false;

        std::shared_ptr<Format> formatret;
        if (format)
        {
            std::shared_ptr<ProxLocation> pLocation;
            formatret = Format::getByFormatType(format->getType());
            formatret->unSerialize(format->serialize(), "");
            unsigned int dataLengthBits = formatret->getDataLength();
            unsigned int atrLengthBits = static_cast<unsigned int>(getChip()->getChipIdentifier().size() * 8);
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
                pLocation->bit = static_cast<int>(atrLengthBits - dataLengthBits); //raw wiegand

				int historBytesSize = getChip()->getChipIdentifier()[1] & 0x0f;
				if ((getChip()->getChipIdentifier().size() - historBytesSize - 1) > 0
					&& getChip()->getChipIdentifier()[getChip()->getChipIdentifier().size() - historBytesSize - 1] == 0x00) //Check Prox Format
				{
					pLocation->bit -= 8; //skip TCK
					pLocation->bit = static_cast<unsigned char>(pLocation->bit / 8) * 8;
				}
            }

            if (pLocation)
            {
                if (dataLengthBits > 0)
                {
                    size_t length = (dataLengthBits + 7) / 8;
					BitsetStream formatBuf;
                    try
                    {
						formatBuf.concat(getChip()->getChipIdentifier(), static_cast<unsigned int>(pLocation->bit), dataLengthBits);
						formatret->setLinearData(formatBuf.getData());
                        ret = true;
                    }
                    catch (std::exception&)
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

    bool ProxAccessControlCardService::writeFormat(std::shared_ptr<Format> /*format*/, std::shared_ptr<Location> /*location*/, std::shared_ptr<AccessInfo> /*aiToUse*/, std::shared_ptr<AccessInfo> /*aiToWrite*/)
    {
        return false;
    }

    std::vector<unsigned char> ProxAccessControlCardService::getPACSBits()
    {
        return getChip()->getChipIdentifier();
    }

	FormatList ProxAccessControlCardService::getSupportedFormatList()
	{
		FormatList list = AccessControlCardService::getHIDWiegandFormatList();
		list.push_back(std::shared_ptr<RawFormat>(new RawFormat()));
		return list;
	}
}