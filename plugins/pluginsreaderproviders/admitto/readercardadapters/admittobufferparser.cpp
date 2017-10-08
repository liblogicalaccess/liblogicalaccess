/**
 * \file admittobufferparser.cpp
 * \author Adrien J. <adrien.jund@islog.com>
 * \brief Promag buffer parser.
 */

#include "admittobufferparser.hpp"
#include "admittoreadercardadapter.hpp"

namespace logicalaccess
{
    ByteVector AdmittoBufferParser::getValidBuffer(boost::circular_buffer<unsigned char>& circular_buffer)
    {
        ByteVector result;

        if (circular_buffer.size() >= 2)
        {
            for (size_t i = 1; i < circular_buffer.size(); ++i)
            {
                if (circular_buffer[i] == AdmittoReaderCardAdapter::CR && circular_buffer[i + 1] == AdmittoReaderCardAdapter::LF)
                {
                    result.assign(circular_buffer.begin(), circular_buffer.begin() + i + 2);
                    circular_buffer.erase(circular_buffer.begin(), circular_buffer.begin() + i + 2);
                    break;
                }
            }
        }
        return result;
    }
}