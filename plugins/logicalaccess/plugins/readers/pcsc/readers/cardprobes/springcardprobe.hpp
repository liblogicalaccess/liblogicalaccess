#pragma once

#include <logicalaccess/plugins/readers/pcsc/readers/cardprobes/pcsccardprobe.hpp>

namespace logicalaccess
{
class LLA_READERS_PCSC_API SpringCardProbe : public PCSCCardProbe
{
  public:
    explicit SpringCardProbe(ReaderUnit *ru);

    bool maybe_mifare_classic() override;
};
}
