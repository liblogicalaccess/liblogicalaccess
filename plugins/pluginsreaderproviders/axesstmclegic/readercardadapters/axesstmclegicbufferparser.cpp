/**
 * \file axesstmclegicbufferparser.cpp
 * \author Adrien J. <adrien.jund@islog.com>
 * \brief Promag buffer parser.
 */

#include "axesstmclegicbufferparser.hpp"
#include "axesstmclegicreadercardadapter.hpp"

namespace logicalaccess
{
    std::vector<unsigned char> AxessTMCLegicBufferParser::getValidBuffer(boost::circular_buffer<unsigned char>& circular_buffer)
    {
        std::vector<unsigned char> result;

        if (circular_buffer.size() >= 7)
        {
            unsigned short messageSize = circular_buffer[0];
            if (circular_buffer.size() >= static_cast<unsigned short>(messageSize + 1))
            {
                result.assign(circular_buffer.begin(), circular_buffer.begin() + messageSize + 1);
                circular_buffer.erase(circular_buffer.begin(), circular_buffer.begin() + messageSize + 1);
            }
        }
        return result;
    }
}