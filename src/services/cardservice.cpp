/**
 * \file cardservice.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Card service.
 */

#include "logicalaccess/services/cardservice.hpp"
#include "logicalaccess/cards/chip.hpp"

namespace logicalaccess
{
	CardService::CardService(std::shared_ptr<Chip> chip, CardServiceType serviceType) : d_chip(chip), d_serviceType(serviceType)
    {
    }

	CardServiceType CardService::getServiceType() const
	{
		return d_serviceType;
	}
}