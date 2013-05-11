/**
 * \file GenericTagCardProvider.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Generic tag card card provider.
 */

#include "GenericTagCardProvider.h"
#include "GenericTagChip.h"
#include "GenericTagAccessControlCardService.h"

namespace logicalaccess
{
	GenericTagCardProvider::GenericTagCardProvider()
		: CardProvider()
	{

	}

	GenericTagCardProvider::~GenericTagCardProvider()
	{

	}

	bool GenericTagCardProvider::erase()
	{
		return false;
	}

	bool GenericTagCardProvider::erase(boost::shared_ptr<Location> /*location*/, boost::shared_ptr<AccessInfo> /*aiToUse*/)
	{
		return false;
	}
	
	bool GenericTagCardProvider::writeData(boost::shared_ptr<Location> /*location*/, boost::shared_ptr<AccessInfo> /*aiToUse*/, boost::shared_ptr<AccessInfo> /*aiToWrite*/, const void* /*data*/, size_t /*dataLength*/, CardBehavior /*behaviorFlags*/)
	{
		return false;
	}

	bool GenericTagCardProvider::readData(boost::shared_ptr<Location> /*location*/, boost::shared_ptr<AccessInfo> /*aiToUse*/, void* data, size_t dataLength, CardBehavior /*behaviorFlags*/)
	{		
		std::vector<unsigned char> identifier = getChip()->getChipIdentifier();

		if (data != NULL && dataLength <= identifier.size())
		{
			memcpy(data, &identifier[0], dataLength);
			return true;
		}

		return false;
	}

	size_t GenericTagCardProvider::readDataHeader(boost::shared_ptr<Location> /*location*/, boost::shared_ptr<AccessInfo> /*aiToUse*/, void* /*data*/, size_t /*dataLength*/)
	{
		return 0;
	}

	boost::shared_ptr<CardService> GenericTagCardProvider::getService(CardServiceType serviceType)
	{
		boost::shared_ptr<CardService> service;

		switch (serviceType)
		{
		case CST_ACCESS_CONTROL:
			{
				service.reset(new GenericTagAccessControlCardService(shared_from_this()));
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
