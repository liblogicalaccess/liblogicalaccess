#include <logicalaccess/services/uidchanger/uidchangerservice.hpp>

using namespace logicalaccess;

UIDChangerCardService::UIDChangerCardService(std::shared_ptr<Chip> chip)
    : CardService(chip, CST_UID_CHANGER)
{
}
