/**
 * \file gunnebobufferparser.hpp
 * \author Adrien J. <adrien.jund@islog.com>
 * \brief Gunnebo Buffer Parser.
 */

#pragma once

#include <logicalaccess/readerproviders/circularbufferparser.hpp>
#include <logicalaccess/plugins/readers/gunnebo/lla_readers_gunnebo_api.hpp>
#include <string>
#include <vector>

namespace logicalaccess
{
class LLA_READERS_GUNNEBO_API GunneboBufferParser : public CircularBufferParser
{
  public:
    GunneboBufferParser();

    virtual ~GunneboBufferParser() = default;

    ByteVector
    getValidBuffer(boost::circular_buffer<unsigned char> &circular_buffer) override;
};
}
