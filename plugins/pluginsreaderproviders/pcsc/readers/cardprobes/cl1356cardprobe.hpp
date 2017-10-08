#pragma once

#include "pcsccardprobe.hpp"

namespace logicalaccess
{
class LIBLOGICALACCESS_API CL1356CardProbe : public PCSCCardProbe
{
  public:
	explicit CL1356CardProbe(ReaderUnit *ru);

	bool maybe_mifare_classic() override;
};
}
