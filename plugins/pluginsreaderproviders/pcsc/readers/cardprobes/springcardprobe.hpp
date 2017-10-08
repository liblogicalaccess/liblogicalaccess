#pragma once

#include "pcsccardprobe.hpp"

namespace logicalaccess
{
class LIBLOGICALACCESS_API SpringCardProbe : public PCSCCardProbe
{
  public:
	explicit SpringCardProbe(ReaderUnit *ru);

	bool maybe_mifare_classic() override;
};
}
