#pragma once

#include "logicalaccess/services/uidchanger/uidchangerservice.hpp"
#include <logicalaccess/services/storage/storagecardservice.hpp>

namespace logicalaccess
{
/**
 * This service changes the UID of some MifareUltralight cards.
 *
 * This service operates at high-level, and while it doesn't work against
 * some reader (OK5321) it consist of normal block writing operation.
 */
class LIBLOGICALACCESS_API MifareUltralightUIDChangerService : public UIDChangerService
{
  public:
    /**
     * This UID changer needs a Storage service to perform
     * write operation against the card.
     */
    MifareUltralightUIDChangerService(std::shared_ptr<StorageCardService> storage);

    virtual void changeUID(const std::vector<uint8_t> &new_uid) override;

  private:
    std::shared_ptr<StorageCardService> storage_;
};
}
