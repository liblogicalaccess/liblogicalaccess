/**
 * \file osdpbufferparser.hpp
 * \author Adrien J. <adrien.jund@islog.com>
 * \brief OSDP Buffer Parser.
 */

#ifndef OSDPBUFFERPARSER_HPP
#define OSDPBUFFERPARSER_HPP

#include "logicalaccess/readerproviders/circularbufferparser.hpp"

#include <string>
#include <vector>

namespace logicalaccess
{
    class LIBLOGICALACCESS_API OSDPBufferParser : public CircularBufferParser
    {
    public:
        OSDPBufferParser() {};

        virtual ~OSDPBufferParser() {};

        virtual std::vector<unsigned char> getValidBuffer(boost::circular_buffer<unsigned char>& circular_buffer);
    };
}

#endif /* OSDPBUFFERPARSER_HPP */