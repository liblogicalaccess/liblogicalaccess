/**
 * \file axesstmc13bufferparser.hpp
 * \author Adrien J. <adrien.jund@islog.com>
 * \brief AxessTMC13 Buffer Parser.
 */

#ifndef AXESSTMC13BUFFERPARSER_HPP
#define AXESSTMC13BUFFERPARSER_HPP

#include <logicalaccess/readerproviders/circularbufferparser.hpp>
#include <logicalaccess/plugins/readers/axesstmc13/lla_readers_axesstmc13_api.hpp>
#include <string>
#include <vector>

namespace logicalaccess
{
class LLA_READERS_AXESSTMC13_API AxessTMC13BufferParser : public CircularBufferParser
{
  public:
    AxessTMC13BufferParser()
    {
    }

    virtual ~AxessTMC13BufferParser()
    {
    }

    ByteVector
    getValidBuffer(boost::circular_buffer<unsigned char> &circular_buffer) override;
};
}

#endif /* AXESSTMC13BUFFERPARSER_HPP */