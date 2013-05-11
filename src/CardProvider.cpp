/**
 * \file CardProvider.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Card provider.
 */

#include "logicalaccess/Cards/CardProvider.h"
#include "logicalaccess/Services/AccessControl/CardsFormatComposite.h"
#include "logicalaccess/Cards/ReaderCardAdapter.h"


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

