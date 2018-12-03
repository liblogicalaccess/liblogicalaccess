/**
 * \file gigatmsbufferparser.hpp
 * \author Adrien J. <adrien.jund@islog.com>
 * \brief GIGA-TMS Buffer Parser.
 */

#ifndef GIGATMSBUFFERPARSER_HPP
#define GIGATMSBUFFERPARSER_HPP

#include <logicalaccess/readerproviders/circularbufferparser.hpp>
#include <logicalaccess/plugins/readers/gigatms/lla_readers_gigatms_api.hpp>
#include <string>
#include <vector>

namespace logicalaccess
{
class LLA_READERS_GIGATMS_API GigaTMSBufferParser : public CircularBufferParser
{
  public:
    GigaTMSBufferParser()
    {
    }

    virtual ~GigaTMSBufferParser()
    {
    }

    ByteVector
    getValidBuffer(boost::circular_buffer<unsigned char> &circular_buffer) override;
};
}

#endif /* GIGATMSBUFFERPARSER_HPP */