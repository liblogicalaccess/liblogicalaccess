/**
 * \file elatecbufferparser.hpp
 * \author Adrien J. <adrien.jund@islog.com>
 * \brief Elatec Buffer Parser.
 */

#ifndef ELATECBUFFERPARSER_HPP
#define ELATECBUFFERPARSER_HPP

#include <logicalaccess/readerproviders/circularbufferparser.hpp>
#include <logicalaccess/plugins/readers/elatec/lla_readers_elatec_api.hpp>
#include <string>
#include <vector>

namespace logicalaccess
{
class LLA_READERS_ELATEC_API ElatecBufferParser : public CircularBufferParser
{
  public:
    ElatecBufferParser()
    {
    }

    virtual ~ElatecBufferParser()
    {
    }

    ByteVector
    getValidBuffer(boost::circular_buffer<unsigned char> &circular_buffer) override;
};
}

#endif /* ELATECBUFFERPARSER_HPP */