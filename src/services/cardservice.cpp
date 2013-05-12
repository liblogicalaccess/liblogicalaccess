/**
 * \file cardservice.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Card service.
 */

#include "logicalaccess/services/cardservice.hpp"
#include "logicalaccess/cards/cardprovider.hpp"


namespace logicalaccess
{
	CardService::CardService(boost::shared_ptr<CardProvider> cardProvider)
	{
		d_cardProvider = cardProvider;
	}

	boost::shared_ptr<CardProvider> CardService::getCardProvider()
	{
		return d_cardProvider;
	}
}

