/**
 * \file axesstmclegicbufferparser.hpp
 * \author Adrien J. <adrien.jund@islog.com>
 * \brief AxessTMCLegic Buffer Parser.
 */

#ifndef AXESSTMCLEGICBUFFERPARSER_HPP
#define AXESSTMCLEGICBUFFERPARSER_HPP

#include <logicalaccess/readerproviders/circularbufferparser.hpp>

#include <string>
#include <vector>

namespace logicalaccess
{
class LIBLOGICALACCESS_API AxessTMCLegicBufferParser : public CircularBufferParser
{
  public:
    AxessTMCLegicBufferParser()
    {
    }

    virtual ~AxessTMCLegicBufferParser()
    {
    }

    ByteVector
    getValidBuffer(boost::circular_buffer<unsigned char> &circular_buffer) override;
};
}

#endif /* AXESSTMCLEGICBUFFERPARSER_HPP */