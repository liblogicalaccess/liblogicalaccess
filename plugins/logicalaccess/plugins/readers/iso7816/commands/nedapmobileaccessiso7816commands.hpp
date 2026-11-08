#pragma once

#include <logicalaccess/plugins/cards/nedapmobileaccess/nedapmobileaccesscommands.hpp>
#include <logicalaccess/plugins/readers/iso7816/commands/iso7816iso7816commands.hpp>
#include <logicalaccess/plugins/readers/iso7816/lla_readers_iso7816_api.hpp>

namespace logicalaccess
{
#define CMD_NEDAP_MOBILE_ACCESS_ISO7816 "NedapMobileAccessISO7816"

class LLA_READERS_ISO7816_API NedapMobileAccessISO7816Commands
    : public NedapMobileAccessCommands
{
  public:
    NedapMobileAccessISO7816Commands();

    std::shared_ptr<ISO7816Commands> getISO7816Commands() const override;
    ByteVector selectApplication(const ByteVector &dfName) override;

  protected:
    ISO7816Response sendNativeCommand(unsigned char instruction,
                                      const ByteVector &data) const override;
};
}
