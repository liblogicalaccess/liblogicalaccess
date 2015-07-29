/**
 * \file gunnebobufferparser.cpp
 * \author Adrien J. <adrien.jund@islog.com>
 * \brief Gunnebo buffer parser.
 */

#include "gunnebobufferparser.hpp"
#include "gunneboreadercardadapter.hpp"

namespace logicalaccess
{
    GunneboBufferParser::GunneboBufferParser()
    {
        d_checksum = true;
    }

    void GunneboBufferParser::setChecksum(bool checksum)
    {
        d_checksum = checksum;
    }

    std::vector<unsigned char> GunneboBufferParser::getValidBuffer(boost::circular_buffer<unsigned char>& circular_buffer)
    {
        std::vector<unsigned char> result;

        size_t foolen = d_checksum ? 2 : 1;
        if (circular_buffer.size() >= (1 + foolen) && circular_buffer[0] == GunneboReaderCardAdapter::STX)
        {
            for (size_t i = 1; i < circular_buffer.size(); ++i)
            {
                if (circular_buffer[i] == GunneboReaderCardAdapter::ETX)
                {
                    if (circular_buffer.size() >= i + foolen)
                    {
                        result.assign(circular_buffer.begin(), circular_buffer.begin() + i + foolen);
                        circular_buffer.erase(circular_buffer.begin(), circular_buffer.begin() + i + foolen);
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