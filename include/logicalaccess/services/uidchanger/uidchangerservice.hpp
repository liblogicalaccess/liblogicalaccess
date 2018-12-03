#pragma once

#include <logicalaccess/services/cardservice.hpp>
#include <logicalaccess/lla_fwd.hpp>
#include <vector>

namespace logicalaccess
{
/**
 * This service operate mostly on "backup cards" whose UID
 * can be changed.
 */
class LLA_CORE_API UIDChangerCardService : public CardService
{
  public:
    explicit UIDChangerCardService(std::shared_ptr<Chip> chip);
    virtual ~UIDChangerCardService() = default;

    virtual void changeUID(const ByteVector &new_uid) = 0;
};
}
