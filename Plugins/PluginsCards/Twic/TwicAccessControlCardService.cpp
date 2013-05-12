/**
 * \file twicaccesscontrolcardservice.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Twic Access Control Card service.
 */

#include "twicaccesscontrolcardservice.hpp"
#include "twiccardprovider.hpp"
#include "twicchip.hpp"
#include "logicalaccess/cards/readercardadapter.hpp"
#include "logicalaccess/services/accesscontrol/formats/bithelper.hpp"
#include "logicalaccess/services/accesscontrol/formats/fascn200bitformat.hpp"


#ifdef __linux__
#include <cstring>
#endif

namespace logicalaccess
{
	TwicAccessControlCardService::TwicAccessControlCardService(boost::shared_ptr<CardProvider> cardProvider)
		: AccessControlCardService(cardProvider)
	{
		
	}

	TwicAccessControlCardService::~TwicAccessControlCardService()
	{
	}

	boost::shared_ptr<TwicCardProvider> TwicAccessControlCardService::getTwicCardProvider()
	{
		return boost::dynamic_pointer_cast<TwicCardProvider>(getCardProvider());
	}

	boost::shared_ptr<Format> TwicAccessControlCardService::readFormat(boost::shared_ptr<Format> format, boost::shared_ptr<Location> location, boost::shared_ptr<AccessInfo> /*aiToUse*/)
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
			formatret = LOGICALACCESS::Format::getByFormatType(format->getType());
			formatret->unSerialize(format->serialize(), "");
		}
		else
		{
			formatret.reset(new FASCN200BitFormat());
		}

		boost::shared_ptr<TwicLocation> pLocation;
		if (location)
		{
			pLocation = boost::dynamic_pointer_cast<TwicLocation>(location);
		}
		else
		{
			if (boost::dynamic_pointer_cast<FASCN200BitFormat>(formatret))
			{
				pLocation.reset(new TwicLocation());
				pLocation->dataObject = 0x5FC104;
				pLocation->tag = 0x30;
			}
		}				
			
		if (pLocation)
		{
			size_t length = (formatret->getDataLength() + 7) / 8;
			unsigned char *formatBuf = new unsigned char[length];
			memset(formatBuf, 0x00, length);
			try
			{
				if (getCardProvider()->readData(pLocation, boost::shared_ptr<AccessInfo>(), formatBuf, length, CB_DEFAULT))
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

		if (!ret)
		{
			formatret.reset();
		}

		return formatret;
	}

	bool TwicAccessControlCardService::writeFormat(boost::shared_ptr<Format> /*format*/, boost::shared_ptr<Location> /*location*/, boost::shared_ptr<AccessInfo> /*aiToUse*/, boost::shared_ptr<AccessInfo> /*aiToWrite*/)
	{
		return false;
	}
}

