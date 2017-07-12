#include "epasschip.hpp"
#include "epassidentityservice.hpp"
#include "epassaccessinfo.hpp"

using namespace logicalaccess;

std::shared_ptr<EPassCommands> EPassChip::getEPassCommands()
{
    return std::dynamic_pointer_cast<EPassCommands>(getCommands());
}

EPassChip::EPassChip()
    : ISO7816Chip("EPass")
{
}

std::shared_ptr<CardService> EPassChip::getService(CardServiceType serviceType)
{
    if (serviceType == CST_IDENTITY)
        return std::make_shared<EPassIdentityService>(shared_from_this());
    return ISO7816Chip::getService(serviceType);
}

std::shared_ptr<AccessInfo> EPassChip::createAccessInfo() const
{
	return std::make_shared<EPassAccessInfo>();
}
