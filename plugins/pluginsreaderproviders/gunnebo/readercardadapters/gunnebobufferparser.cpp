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
        
    }

    ByteVector GunneboBufferParser::getValidBuffer(boost::circular_buffer<unsigned char>& circular_buffer)
    {
        ByteVector result;

        if (circular_buffer.size() >= 3 && circular_buffer[0] == GunneboReaderCardAdapter::STX)
        {
            // Check if STid or Gunnebo reader
            size_t foolen = (circular_buffer[1] == 0x31 && circular_buffer[2] == 0x46) ? 1 : 2;
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
	                LOG(LogLevel::COMS) << "ETX found but no checksum bytes.";
                }
            }
        }
        return result;
    }
}