/**
 * \file circularbufferparser.cpp
 * \author Adrien J. <adrien.jund@islog.com>
 * \brief A CircularBufferParser class.
 */

#include "logicalaccess/readerproviders/circularbufferparser.hpp"

namespace logicalaccess
{
    ByteVector CircularBufferParser::getValidBuffer(boost::circular_buffer<unsigned char>& circular_buffer)
    {
        ByteVector result(circular_buffer.begin(), circular_buffer.end());
        circular_buffer.clear();
        return result;
    }
}