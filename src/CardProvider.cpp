/**
 * \file cardprovider.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Card provider.
 */

#include "logicalaccess/cards/cardprovider.hpp"
#include "logicalaccess/services/accesscontrol/cardsformatcomposite.hpp"
#include "logicalaccess/cards/readercardadapter.hpp"


namespace logicalaccess
{
	CardProvider::CardProvider()
	{
	}	

	CardProvider::~CardProvider()
	{
	}

	boost::shared_ptr<CardService> CardProvider::getService(CardServiceType /*serviceType*/)
	{
		return boost::shared_ptr<CardService>();
	}
}

