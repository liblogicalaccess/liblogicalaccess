/**
 * \file promagbufferparser.hpp
 * \author Adrien J. <adrien.jund@islog.com>
 * \brief Promag Buffer Parser.
 */

#ifndef PROMAGBUFFERPARSER_HPP
#define PROMAGBUFFERPARSER_HPP

#include <logicalaccess/readerproviders/circularbufferparser.hpp>
#include <logicalaccess/plugins/readers/promag/lla_readers_promag_api.hpp>
#include <string>
#include <vector>

namespace logicalaccess
{
class LLA_READERS_PROMAG_API PromagBufferParser : public CircularBufferParser
{
  public:
    PromagBufferParser()
    {
    }

    virtual ~PromagBufferParser()
    {
    }

    ByteVector
    getValidBuffer(boost::circular_buffer<unsigned char> &circular_buffer) override;
};
}

#endif /* PROMAGBUFFERPARSER_HPP */