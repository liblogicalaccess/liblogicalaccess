/**
 * \file osdpbufferparser.hpp
 * \author Adrien J. <adrien.jund@islog.com>
 * \brief OSDP Buffer Parser.
 */

#ifndef OSDPBUFFERPARSER_HPP
#define OSDPBUFFERPARSER_HPP

#include <logicalaccess/readerproviders/circularbufferparser.hpp>
#include <logicalaccess/plugins/readers/osdp/lla_readers_osdp_api.hpp>
#include <string>
#include <vector>

namespace logicalaccess
{
class LLA_READERS_OSDP_API OSDPBufferParser : public CircularBufferParser
{
  public:
    OSDPBufferParser()
    {
    }

    virtual ~OSDPBufferParser()
    {
    }

    ByteVector
    getValidBuffer(boost::circular_buffer<unsigned char> &circular_buffer) override;
};
}

#endif /* OSDPBUFFERPARSER_HPP */