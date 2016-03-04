#include "epass_chip.hpp"
#include "epass_identity_service.hpp"

using namespace logicalaccess;

std::shared_ptr<EPassCommand> EPassChip::getEPassCommand()
{
    return std::dynamic_pointer_cast<EPassCommand>(getCommands());
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
