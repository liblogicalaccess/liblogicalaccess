#pragma once

#include "logicalaccess/services/accesscontrol/accesscontrolcardservice.hpp"

namespace logicalaccess
{
/**
 * This service is a implementation of the AccessControl service
 * for the STidPRG reader and writable 13.56khz RFID card.
 *
 * It allows the client to configure a card to emulate various
 * format.
 */
class LIBLOGICALACCESS_API STidPRGProxAccessControl : public AccessControlCardService
{
  public:
    explicit STidPRGProxAccessControl(std::shared_ptr<Chip> chip);

    bool writeFormat(std::shared_ptr<Format> format, std::shared_ptr<Location> location,
                     std::shared_ptr<AccessInfo> aiToUse,
                     std::shared_ptr<AccessInfo> aiToWrite) override;
};
}
