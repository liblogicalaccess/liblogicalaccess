/**
 * \file stidstrreaderbufferparser.hpp
 * \author Adrien J. <adrien.jund@islog.com>
 * \brief STidSTR Buffer Parser.
 */

#ifndef STIDSTRBUFFERPARSER_HPP
#define STIDSTRBUFFERPARSER_HPP

#include <logicalaccess/readerproviders/circularbufferparser.hpp>
#include <logicalaccess/plugins/readers/stidstr/lla_readers_stidstr_api.hpp>
#include <string>
#include <vector>

namespace logicalaccess
{
class LLA_READERS_STIDSTR_API STidSTRBufferParser : public CircularBufferParser
{
  public:
    STidSTRBufferParser()
    {
    }

    virtual ~STidSTRBufferParser()
    {
    }

    ByteVector
    getValidBuffer(boost::circular_buffer<unsigned char> &circular_buffer) override;
};
}

#endif /* STIDSTRBUFFERPARSER_HPP */