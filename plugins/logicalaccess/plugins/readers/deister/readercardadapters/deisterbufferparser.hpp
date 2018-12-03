/**
 * \file deisterbufferparser.hpp
 * \author Adrien J. <adrien.jund@islog.com>
 * \brief Deister Buffer Parser.
 */

#ifndef DEISTERBUFFERPARSER_HPP
#define DEISTERBUFFERPARSER_HPP

#include <logicalaccess/readerproviders/circularbufferparser.hpp>
#include <logicalaccess/plugins/readers/deister/lla_readers_deister_api.hpp>
#include <string>
#include <vector>

namespace logicalaccess
{
class LLA_READERS_DEISTER_API DeisterBufferParser : public CircularBufferParser
{
  public:
    DeisterBufferParser()
    {
    }

    virtual ~DeisterBufferParser()
    {
    }

    ByteVector
    getValidBuffer(boost::circular_buffer<unsigned char> &circular_buffer) override;
};
}

#endif /* DEISTERBUFFERPARSER_HPP */