/**
 * \file ProxAccessControlCardService.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Prox Access Control Card service.
 */

#include "ProxAccessControlCardService.h"
#include "ProxCardProvider.h"
#include "logicalaccess/Cards/ReaderCardAdapter.h"



#ifdef __linux__
#include <cstring>
#endif

namespace LOGICALACCESS
{
	ProxAccessControlCardService::ProxAccessControlCardService(boost::shared_ptr<CardProvider> cardProvider)
		: AccessControlCardService(cardProvider)
	{
		
	}

	ProxAccessControlCardService::~ProxAccessControlCardService()
	{
	}

	boost::shared_ptr<ProxCardProvider> ProxAccessControlCardService::getProxCardProvider()
	{
		return boost::dynamic_pointer_cast<ProxCardProvider>(getCardProvider());
	}

	boost::shared_ptr<Format> ProxAccessControlCardService::readFormat(boost::shared_ptr<Format> format, boost::shared_ptr<Location> location, boost::shared_ptr<AccessInfo> /*aiToUse*/)
	{
		bool ret = false;

#ifdef _LICENSE_SYSTEM
		if (!d_license.hasReadFormatAccess())
		{
			THROW_EXCEPTION_WITH_LOG(LibLOGICALACCESSException, EXCEPTION_MSG_LICENSENOACCESS);
		}
#endif

		boost::shared_ptr<Format> formatret;
		if (format)
		{
			boost::shared_ptr<ProxLocation> pLocation;
			formatret = LOGICALACCESS::Format::getByFormatType(format->getType());
			formatret->unSerialize(format->serialize(), "");
			unsigned int dataLengthBits = formatret->getDataLength();
			unsigned int atrLengthBits = getProxCardProvider()->readData(pLocation, NULL, 0, dataLengthBits);			
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
						if (getProxCardProvider()->readData(pLocation, formatBuf, length, dataLengthBits))
						{
							formatret->setLinearData(formatBuf, length);
							ret = true;
						}
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
		std::vector<unsigned char> pacsBits;
		unsigned char atr[255];
		memset(atr, 0x00, sizeof(atr));

		unsigned int atrLengthBits = getProxCardProvider()->readData(boost::shared_ptr<ProxLocation>(), NULL, 0, 0);
		if (atrLengthBits > (2*8))
		{
			getProxCardProvider()->readData(boost::shared_ptr<ProxLocation>(), atr, sizeof(atr), atrLengthBits);
			pacsBits.insert(pacsBits.end(), atr + 2, atr + ((atrLengthBits + 7) / 8));
		}


		return pacsBits;
	}
}

