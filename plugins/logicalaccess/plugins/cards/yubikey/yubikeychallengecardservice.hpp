/**
 * \file yubikeychallengecardservice.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Yubikey Challenge-Response Card service.
 */

#ifndef LOGICALACCESS_YUBIKEYCHALLENGECARDSERVICE_HPP
#define LOGICALACCESS_YUBIKEYCHALLENGECARDSERVICE_HPP

#include <logicalaccess/plugins/cards/yubikey/yubikeychip.hpp>
#include <logicalaccess/services/challenge/challengecardservice.hpp>

namespace logicalaccess
{
#define YUBIKEY_CHALLENGE_CARDSERVICE "YubikeyChallenge"

/**
 * \brief The Yubikey challenge-response card service class.
 */
class LLA_CARDS_YUBIKEY_API YubikeyChallengeCardService : public ChallengeCardService
{
  public:
    /**
     * \brief Constructor.
     * \param chip The associated chip.
     */
    explicit YubikeyChallengeCardService(std::shared_ptr<Chip> chip);

    /**
     * \brief Destructor.
     */
    ~YubikeyChallengeCardService();
    
    std::string getCSType() override;
    
    ByteVector getChallenge() override;
    
    ByteVector getResponse(const ByteVector &challenge, uint8_t slot = 0) override;

  protected:
    std::shared_ptr<YubikeyChip> getYubikeyChip() const
    {
        return std::dynamic_pointer_cast<YubikeyChip>(getChip());
    }
};
}

#endif