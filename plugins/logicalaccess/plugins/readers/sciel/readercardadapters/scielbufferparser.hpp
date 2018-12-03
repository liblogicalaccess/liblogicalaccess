/**
 * \file scielbufferparser.hpp
 * \author Adrien J. <adrien.jund@islog.com>
 * \brief Sciel Buffer Parser.
 */

#ifndef SCIELBUFFERPARSER_HPP
#define SCIELBUFFERPARSER_HPP

#include <logicalaccess/readerproviders/circularbufferparser.hpp>
#include <logicalaccess/plugins/readers/sciel/lla_readers_sciel_api.hpp>
#include <string>
#include <vector>

namespace logicalaccess
{
class LLA_READERS_SCIEL_API ScielBufferParser : public CircularBufferParser
{
  public:
    ScielBufferParser()
    {
    }

    virtual ~ScielBufferParser()
    {
    }

    ByteVector
    getValidBuffer(boost::circular_buffer<unsigned char> &circular_buffer) override;
};
}

#endif /* SCIELBUFFERPARSER_HPP */