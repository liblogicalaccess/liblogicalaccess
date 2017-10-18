#pragma once

#include "logicalaccess/services/cardservice.hpp"
#include "logicalaccess/lla_fwd.hpp"
#include <vector>

namespace logicalaccess
{
/**
 * This service operate mostly on "backup cards" whose UID
 * can be changed.
 */
class LIBLOGICALACCESS_API UIDChangerService : public CardService
{
  public:
    explicit UIDChangerService(std::shared_ptr<Chip> chip);
    virtual ~UIDChangerService() = default;

    virtual void changeUID(const ByteVector &new_uid) = 0;
};
}
