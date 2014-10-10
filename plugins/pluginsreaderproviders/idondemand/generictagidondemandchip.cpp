/**
 * \file generictagidondemandchip.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Generic tag IdOnDemand chip.
 */

#include "generictagidondemandchip.hpp"
#include "commands/generictagidondemandaccesscontrolcardservice.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>

#include <cstring>

namespace logicalaccess
{
    GenericTagIdOnDemandChip::GenericTagIdOnDemandChip()
        : GenericTagChip()
    {
    }

    GenericTagIdOnDemandChip::~GenericTagIdOnDemandChip()
    {
    }

    boost::shared_ptr<CardService> GenericTagIdOnDemandChip::getService(CardServiceType serviceType)
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
        case CST_STORAGE:
            break;
        }

        if (!service)
        {
            service = Chip::getService(serviceType);
        }

        return service;
    }
}