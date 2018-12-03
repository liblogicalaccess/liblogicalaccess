/**
 * \file admittobufferparser.hpp
 * \author Adrien J. <adrien.jund@islog.com>
 * \brief Admitto Buffer Parser.
 */

#ifndef ADMITTOBUFFERPARSER_HPP
#define ADMITTOBUFFERPARSER_HPP

#include <logicalaccess/readerproviders/circularbufferparser.hpp>
#include <logicalaccess/plugins/readers/admitto/lla_readers_admitto_api.hpp>
#include <string>
#include <vector>

namespace logicalaccess
{
class LLA_READERS_ADMITTO_API AdmittoBufferParser : public CircularBufferParser
{
  public:
    AdmittoBufferParser()
    {
    }

    virtual ~AdmittoBufferParser()
    {
    }

    ByteVector
    getValidBuffer(boost::circular_buffer<unsigned char> &circular_buffer) override;
};
}

#endif /* ADMITTOBUFFERPARSER_HPP */