/**
 * \file gunnebobufferparser.cpp
 * \author Adrien J. <adrien.jund@islog.com>
 * \brief Gunnebo buffer parser.
 */

#include "gunnebobufferparser.hpp"
#include "gunneboreadercardadapter.hpp"

namespace logicalaccess
{
    std::vector<unsigned char> GunneboBufferParser::getValidBuffer(boost::circular_buffer<unsigned char>& circular_buffer)
    {
        std::vector<unsigned char> result;

        if (circular_buffer.size() >= 3 && circular_buffer[0] == GunneboReaderCardAdapter::STX)
        {
            for (size_t i = 1; i < circular_buffer.size(); ++i)
            {
                if (circular_buffer[i] == GunneboReaderCardAdapter::ETX)
                {
                    if (circular_buffer.size() >= i + 2)
                    {
                        result.assign(circular_buffer.begin(), circular_buffer.begin() + i + 2);
                        circular_buffer.erase(circular_buffer.begin(),
                                              circular_buffer.begin() + i + 2);
                        break;
                    }
                    else
                    {
                        LOG(LogLevel::COMS) << "ETX found but no checksum bytes.";
                    }
                }
            }
        }
        return result;
    }
}