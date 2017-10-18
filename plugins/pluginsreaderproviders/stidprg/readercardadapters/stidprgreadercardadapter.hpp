#pragma once

#include "logicalaccess/cards/readercardadapter.hpp"

namespace logicalaccess
{
class LIBLOGICALACCESS_API STidPRGReaderCardAdapter : public ReaderCardAdapter
{

  public:
	ByteVector
    adaptCommand(const ByteVector &command) override;

	ByteVector
    adaptAnswer(const ByteVector &answer) override;

  protected:
	  static uint8_t compute_crc(const ByteVector &cmd);
};
}