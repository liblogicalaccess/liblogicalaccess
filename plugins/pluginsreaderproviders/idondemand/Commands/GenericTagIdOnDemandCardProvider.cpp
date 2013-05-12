/**
 * \file generictagidondemandcardprovider.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Generic tag IdOnDemand card card provider.
 */

#include "generictagidondemandcardprovider.hpp"
#include "generictagidondemandaccesscontrolcardservice.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>

#include <cstring>


namespace logicalaccess
{
	GenericTagIdOnDemandCardProvider::GenericTagIdOnDemandCardProvider()
		: GenericTagCardProvider()
	{
	}

	GenericTagIdOnDemandCardProvider::~GenericTagIdOnDemandCardProvider()
	{
	}

	boost::shared_ptr<CardService> GenericTagIdOnDemandCardProvider::getService(CardServiceType serviceType)
	{
		boost::shared_ptr<CardService> service;

		switch (serviceType)
		{
		case CST_ACCESS_CONTROL:
			{
				service.reset(new GenericTagIdOnDemandAccessControlCardService(shared_from_this()));
			}
			break;
		case CST_NFC_TAG:
		  break;
		}

		if (!service)
		{
			service = GenericTagCardProvider::getService(serviceType);
		}

		return service;
	}
}
