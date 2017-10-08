#pragma once

#include "logicalaccess/readerproviders/circularbufferparser.hpp"

namespace logicalaccess
{
class LIBLOGICALACCESS_API STidPRGBufferParser : public CircularBufferParser
{

  public:
	ByteVector
    getValidBuffer(boost::circular_buffer<unsigned char> &circular_buffer) override;
};
}