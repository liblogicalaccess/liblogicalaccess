/**
 * \file axesstmc13bufferparser.cpp
 * \author Adrien J. <adrien.jund@islog.com>
 * \brief Promag buffer parser.
 */

#include "axesstmc13bufferparser.hpp"
#include "axesstmc13readercardadapter.hpp"

namespace logicalaccess
{
    ByteVector AxessTMC13BufferParser::getValidBuffer(boost::circular_buffer<unsigned char>& circular_buffer)
    {
        ByteVector result;
        for (size_t i = 0; i < circular_buffer.size(); ++i)
        {
            if (circular_buffer[i] == AxessTMC13ReaderCardAdapter::CR)
            {
                result.assign(circular_buffer.begin(), circular_buffer.begin() + i + 1);
                circular_buffer.erase(circular_buffer.begin(), circular_buffer.begin() + i + 1);
                break;
            }
        }
        return result;
    }
}