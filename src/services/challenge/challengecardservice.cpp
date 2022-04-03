#include <logicalaccess/services/challenge/challengecardservice.hpp>

using namespace logicalaccess;

ChallengeCardService::ChallengeCardService(std::shared_ptr<Chip> chip)
    : CardService(chip, CST_CHALLENGE_RESPONSE)
{
}
