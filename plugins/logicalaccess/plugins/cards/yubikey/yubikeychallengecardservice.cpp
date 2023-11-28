/**
 * \file yubikeychallengecardservice.cpp
 * \author Maxime C. <maxime@leosac.com>
 * \brief Yubikey Challenge-Response Card service.
 */

#include <logicalaccess/plugins/cards/yubikey/yubikeychallengecardservice.hpp>
#include <logicalaccess/plugins/cards/yubikey/yubikeychip.hpp>
#include <logicalaccess/plugins/llacommon/logs.hpp>
#include <logicalaccess/myexception.hpp>

#if defined(__unix__)
#include <cstring>
#endif

namespace logicalaccess
{
YubikeyChallengeCardService::YubikeyChallengeCardService(std::shared_ptr<Chip> chip)
    : ChallengeCardService(chip)
{
}

YubikeyChallengeCardService::~YubikeyChallengeCardService()
{
}

std::string YubikeyChallengeCardService::getCSType()
{
    return YUBIKEY_CHALLENGE_CARDSERVICE;
}

ByteVector YubikeyChallengeCardService::getChallenge()
{
    auto cmd = getYubikeyChip()->getYubikeyCommands();
    EXCEPTION_ASSERT_WITH_LOG(cmd != nullptr,
                              LibLogicalAccessException, "Yubikey Commands is null.");
    return cmd->selectYubikeyOTP();
}

ByteVector YubikeyChallengeCardService::getResponse(const ByteVector& challenge, uint8_t slot)
{
    auto cmd = getYubikeyChip()->getYubikeyCommands();
    EXCEPTION_ASSERT_WITH_LOG(cmd != nullptr,
                              LibLogicalAccessException, "Yubikey Commands is null.");
    YubikeySlot ykslot = SLOT_CHALLENGE_HMAC_1;
    if (slot == 2)
    {
        ykslot = SLOT_CHALLENGE_HMAC_2;
    }
    return cmd->otp_getResponse(ykslot, challenge);
}
}