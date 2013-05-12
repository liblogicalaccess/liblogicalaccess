/**
 * \file proxcardprovider.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Prox card profiles.
 */

#include "proxcardprovider.hpp"

#include "logicalaccess/services/accesscontrol/formats/bithelper.hpp"
#include "proxlocation.hpp"

#include "logicalaccess/services/accesscontrol/formats/wiegand26format.hpp"
#include "logicalaccess/services/accesscontrol/formats/wiegand34format.hpp"
#include "logicalaccess/services/accesscontrol/formats/wiegand34withfacilityformat.hpp"
#include "logicalaccess/services/accesscontrol/formats/wiegand37format.hpp"
#include "logicalaccess/services/accesscontrol/formats/wiegand37withfacilityformat.hpp"
#include "logicalaccess/services/accesscontrol/formats/corporate1000format.hpp"
#include "logicalaccess/services/accesscontrol/formats/dataclockformat.hpp"
#include "logicalaccess/services/accesscontrol/formats/getronik40bitformat.hpp"
#include "logicalaccess/services/accesscontrol/formats/hidhoneywellformat.hpp"
#include "logicalaccess/services/accesscontrol/formats/bariumferritepcscformat.hpp"
#include "logicalaccess/services/accesscontrol/formats/rawformat.hpp"
#include "proxaccesscontrolcardservice.hpp"

namespace logicalaccess
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
