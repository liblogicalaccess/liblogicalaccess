/**
 * \file stidstrreaderbufferparser.cpp
 * \author Adrien J. <adrien.jund@islog.com>
 * \brief STidSTR buffer parser.
 */

#include <logicalaccess/logs.hpp>
#include "stidstrreaderbufferparser.hpp"
#include "logicalaccess/bufferhelper.hpp"

namespace logicalaccess
{
    ByteVector STidSTRBufferParser::getValidBuffer(boost::circular_buffer<unsigned char>& circular_buffer)
    {
        ByteVector result;

        if (circular_buffer.size() >= 7)
        {
            unsigned short messageSize = (circular_buffer[1] << 8) | circular_buffer[2];
            if (circular_buffer.size() >= static_cast<unsigned short>(messageSize + 7))
            {
                result.assign(circular_buffer.begin(), circular_buffer.begin() + messageSize + 7);
                circular_buffer.erase(circular_buffer.begin(), circular_buffer.begin() + messageSize + 7);
                LOG(LogLevel::COMS) << "Header found with the data: " << BufferHelper::getHex(result) << " Remaining on data on circular buffer: " << circular_buffer.size();
            }
            else
                LOG(LogLevel::COMS) << "Header found without the data size expected: " << messageSize;
        }
        return result;
    }
}