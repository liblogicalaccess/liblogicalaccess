/**
 * \file osdpbufferparser.cpp
 * \author Adrien J. <adrien.jund@islog.com>
 * \brief OSDP buffer parser.
 */

#include "osdpbufferparser.hpp"

namespace logicalaccess
{
    std::vector<unsigned char> OSDPBufferParser::getValidBuffer(boost::circular_buffer<unsigned char>& circular_buffer)
    {
        std::vector<unsigned char> result;

        if (circular_buffer.size() >= 6)
        {
            short packetLength = ((circular_buffer[3 + 1] << 8) + circular_buffer[3]);
            if (static_cast<short>(circular_buffer.size()) >= packetLength + 1) // + 1 because of the 0xff
            {
                result.assign(circular_buffer.begin(), circular_buffer.begin() + packetLength + 1);
                circular_buffer.erase(circular_buffer.begin(), circular_buffer.begin() + packetLength + 1);
            }
        }
        return result;
    }
}