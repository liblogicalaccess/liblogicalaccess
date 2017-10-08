/**
 * \file circularbufferparser.hpp
 * \author Adrien J. <adrien.jund@islog.com>
 * \brief A CircularBufferParser class.
 */

#ifndef CIRCULARBUFFERPARSER_HPP
#define CIRCULARBUFFERPARSER_HPP

#include <vector>
#include <boost/circular_buffer.hpp>

#include "logicalaccess/readerproviders/datatransport.hpp"

namespace logicalaccess
{
    class LIBLOGICALACCESS_API CircularBufferParser
    {
    public:
        CircularBufferParser() {}

        virtual ~CircularBufferParser() {}

        virtual ByteVector getValidBuffer(boost::circular_buffer<unsigned char>& circular_buffer);
    };
}

#endif /* CIRCULARBUFFERPARSER_HPP */