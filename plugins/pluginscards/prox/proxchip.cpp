/**
 * \file proxchip.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Prox chip.
 */

#include "proxchip.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>

#include "proxaccesscontrolcardservice.hpp"


namespace logicalaccess
{
	ProxChip::ProxChip()
		: Chip("Prox")
	{
		d_profile.reset(new ProxProfile());
	}

	ProxChip::~ProxChip()
	{
	}	

	boost::shared_ptr<LocationNode> ProxChip::getRootLocationNode()
	{
		boost::shared_ptr<LocationNode> rootNode;
		rootNode.reset(new LocationNode());

		rootNode->setName("HID Prox");

		return rootNode;
	}

	boost::shared_ptr<CardService> ProxChip::getService(CardServiceType serviceType)
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
			service = Chip::getService(serviceType);
		}

		return service;
	}
}
