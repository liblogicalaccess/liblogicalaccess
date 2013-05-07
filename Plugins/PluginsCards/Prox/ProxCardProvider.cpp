/**
 * \file ProxCardProvider.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Prox card profiles.
 */

#include "ProxCardProvider.h"

#include "logicalaccess/Services/AccessControl/Formats/BitHelper.h"
#include "ProxLocation.h"

#include "logicalaccess/Services/AccessControl/Formats/Wiegand26Format.h"
#include "logicalaccess/Services/AccessControl/Formats/Wiegand34Format.h"
#include "logicalaccess/Services/AccessControl/Formats/Wiegand34WithFacilityFormat.h"
#include "logicalaccess/Services/AccessControl/Formats/Wiegand37Format.h"
#include "logicalaccess/Services/AccessControl/Formats/Wiegand37WithFacilityFormat.h"
#include "logicalaccess/Services/AccessControl/Formats/Corporate1000Format.h"
#include "logicalaccess/Services/AccessControl/Formats/DataClockFormat.h"
#include "logicalaccess/Services/AccessControl/Formats/Getronik40BitFormat.h"
#include "logicalaccess/Services/AccessControl/Formats/HIDHoneywellFormat.h"
#include "logicalaccess/Services/AccessControl/Formats/BariumFerritePCSCFormat.h"
#include "logicalaccess/Services/AccessControl/Formats/RawFormat.h"
#include "ProxAccessControlCardService.h"

namespace LOGICALACCESS
{
	ProxCardProvider::ProxCardProvider()
		: CardProvider()
	{

	}

	ProxCardProvider::~ProxCardProvider()
	{

	}

	bool ProxCardProvider::erase()
	{
		return false;
	}

	bool ProxCardProvider::erase(boost::shared_ptr<Location> /*location*/, boost::shared_ptr<AccessInfo> /*aiToUse*/)
	{
		return false;
	}
	
	bool ProxCardProvider::writeData(boost::shared_ptr<Location> /*location*/, boost::shared_ptr<AccessInfo> /*aiToUse*/, boost::shared_ptr<AccessInfo> /*aiToWrite*/, const void* /*data*/, size_t /*dataLength*/, CardBehavior /*behaviorFlags*/)
	{
		return false;
	}

	bool ProxCardProvider::readData(boost::shared_ptr<Location> location, boost::shared_ptr<AccessInfo> /*aiToUse*/, void* data, size_t dataLength, CardBehavior /*behaviorFlags*/)
	{		
		return (readData(boost::dynamic_pointer_cast<ProxLocation>(location), data, dataLength, static_cast<unsigned int>(dataLength * 8)) == static_cast<unsigned int>(dataLength * 8));
	}

	size_t ProxCardProvider::readDataHeader(boost::shared_ptr<Location> /*location*/, boost::shared_ptr<AccessInfo> /*aiToUse*/, void* /*data*/, size_t /*dataLength*/)
	{
		return 0;
	}

	boost::shared_ptr<CardService> ProxCardProvider::getService(CardServiceType serviceType)
	{
		boost::shared_ptr<CardService> service;

		switch (serviceType)
		{
		case CST_ACCESS_CONTROL:
			{
				service.reset(new ProxAccessControlCardService(shared_from_this()));
			}
			break;
		case CST_NFC_TAG:
		  break;
		}

		if (!service)
		{
			service = CardProvider::getService(serviceType);
		}

		return service;
	}
}
