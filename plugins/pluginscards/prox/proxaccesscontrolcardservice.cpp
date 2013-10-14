/**
 * \file proxaccesscontrolcardservice.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Prox Access Control Card service.
 */

#include "proxaccesscontrolcardservice.hpp"
#include "logicalaccess/cards/readercardadapter.hpp"
#include "proxchip.hpp"
#include "logicalaccess/services/accesscontrol/formats/bithelper.hpp"

#if defined(__unix__)
#include <cstring>
#endif

namespace logicalaccess
{
	ProxAccessControlCardService::ProxAccessControlCardService(boost::shared_ptr<Chip> chip)
		: AccessControlCardService(chip)
	{
		
	}

	ProxAccessControlCardService::~ProxAccessControlCardService()
	{
	}

	boost::shared_ptr<Format> ProxAccessControlCardService::readFormat(boost::shared_ptr<Format> format, boost::shared_ptr<Location> location, boost::shared_ptr<AccessInfo> /*aiToUse*/)
	{
		bool ret = false;

		boost::shared_ptr<Format> formatret;
		if (format)
		{
			boost::shared_ptr<ProxLocation> pLocation;
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
				pLocation = boost::dynamic_pointer_cast<ProxLocation>(location);
			}
			else
			{
				pLocation.reset(new ProxLocation());
				// TODO: fix it (signed/unsigned)
				pLocation->bit = static_cast<int>(atrLengthBits - dataLengthBits);
			}
			
			if (pLocation)
			{				
				if (dataLengthBits > 0)
				{
					size_t length = (dataLengthBits + 7) / 8;
					unsigned char *formatBuf = new unsigned char[length];				
					memset(formatBuf, 0x00, length);
					try
					{
						unsigned int pos = 0;
						BitHelper::writeToBit(formatBuf, length, &pos, &getChip()->getChipIdentifier()[0], pLocation->bit, dataLengthBits);
						formatret->setLinearData(formatBuf, length);
						ret = true;
					}
					catch(std::exception&)
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

	bool ProxAccessControlCardService::writeFormat(boost::shared_ptr<Format> /*format*/, boost::shared_ptr<Location> /*location*/, boost::shared_ptr<AccessInfo> /*aiToUse*/, boost::shared_ptr<AccessInfo> /*aiToWrite*/)
	{
		return false;
	}

	std::vector<unsigned char> ProxAccessControlCardService::getPACSBits()
	{
		return getChip()->getChipIdentifier();
	}
}

