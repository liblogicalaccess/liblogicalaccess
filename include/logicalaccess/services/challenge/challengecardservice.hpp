#pragma once

#include <logicalaccess/services/cardservice.hpp>
#include <logicalaccess/lla_fwd.hpp>
#include <vector>

namespace logicalaccess
{
/**
 * This service get a response from a challenge.
 */
class LLA_CORE_API ChallengeCardService : public CardService
{
  public:
    explicit ChallengeCardService(std::shared_ptr<Chip> chip);
    virtual ~ChallengeCardService() = default;

    virtual ByteVector getChallenge() = 0;
    
    virtual ByteVector getResponse(const ByteVector &challenge, uint8_t slot = 0) = 0;
};
}
