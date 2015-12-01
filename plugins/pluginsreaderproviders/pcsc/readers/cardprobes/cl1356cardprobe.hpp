#pragma once

#include "pcsccardprobe.hpp"

namespace logicalaccess
{
class LIBLOGICALACCESS_API CL1356CardProbe : public PCSCCardProbe
{
  public:
    CL1356CardProbe(ReaderUnit *ru);


  public:
    virtual bool maybe_mifare_classic() override;
};
}
