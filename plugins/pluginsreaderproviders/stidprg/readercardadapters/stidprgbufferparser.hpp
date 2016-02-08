#pragma once

#include "logicalaccess/readerproviders/circularbufferparser.hpp"

namespace logicalaccess
{
class STidPRGBufferParser : public CircularBufferParser
{

  public:
    virtual std::vector<unsigned char>
    getValidBuffer(boost::circular_buffer<unsigned char> &circular_buffer) override;
};
}