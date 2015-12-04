#include "logicalaccess/services/uidchanger/uidchangerservice.hpp"

using namespace logicalaccess;

UIDChangerService::UIDChangerService(std::shared_ptr<Chip> chip)
    : CardService(chip)
{
}

CardServiceType UIDChangerService::getServiceType() const
{
    return CST_UID_CHANGER;
}
