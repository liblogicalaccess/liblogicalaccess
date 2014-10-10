/**
 * \file circularbufferparser.cpp
 * \author Adrien J. <adrien.jund@islog.com>
 * \brief A CircularBufferParser class.
 */

#include "logicalaccess/readerproviders/circularbufferparser.hpp"

namespace logicalaccess
{
    std::vector<unsigned char> CircularBufferParser::getValidBuffer(boost::circular_buffer<unsigned char>& circular_buffer)
    {
        std::vector<unsigned char> result(circular_buffer.begin(), circular_buffer.end());
        circular_buffer.clear();
        return result;
    }
}