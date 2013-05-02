/**
 * \file Chip.cpp
 * \author Maxime CHAMLEY <maxime.chamley@islog.eu>
 * \brief Card chip.
 */

#include "logicalaccess/Cards/Chip.h"

#include <fstream>

using std::ofstream;
using std::ifstream;


namespace LOGICALACCESS
{
	Chip::Chip()
	{
		d_cardtype = "UNKNOWN";
		d_powerStatus = CPS_NO_POWER;
		d_receptionLevel = 0;
	}

	Chip::Chip(std::string cardtype)
	{
		d_cardtype = cardtype;
		d_powerStatus = CPS_NO_POWER;
		d_receptionLevel = 0;
	}

	const std::string& Chip::getCardType() const
	{
		return d_cardtype;
	}	

	std::string Chip::getGenericCardType() const
	{
		return d_cardtype;
	}

	void Chip::setCardProvider(boost::shared_ptr<CardProvider> provider)
	{
		d_cardprovider = provider;
		provider->setChip(shared_from_this());
	}

	boost::shared_ptr<LocationNode> Chip::getRootLocationNode()
	{
		boost::shared_ptr<LocationNode> rootNode;
		rootNode.reset(new LocationNode());

		rootNode->setName("Unknown");

		return rootNode;
	}
}

