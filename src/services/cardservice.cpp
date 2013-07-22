/**
 * \file cardservice.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Card service.
 */

#include "logicalaccess/services/cardservice.hpp"
#include "logicalaccess/cards/chip.hpp"


namespace logicalaccess
{
	CardService::CardService(boost::shared_ptr<Chip> chip)
	{
		d_chip = chip;
	}
}

