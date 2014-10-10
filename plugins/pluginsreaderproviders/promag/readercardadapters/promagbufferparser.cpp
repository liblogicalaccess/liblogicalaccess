/**
 * \file promagbufferparser.cpp
 * \author Adrien J. <adrien.jund@islog.com>
 * \brief Promag buffer parser.
 */

#include "promagbufferparser.hpp"
#include "promagreadercardadapter.hpp"

namespace logicalaccess
{
    std::vector<unsigned char> PromagBufferParser::getValidBuffer(boost::circular_buffer<unsigned char>& circular_buffer)
    {
        std::vector<unsigned char> result;

        if (circular_buffer.size() >= 1)
        {
            if (circular_buffer[0] != PromagReaderCardAdapter::ESC && circular_buffer[0] != PromagReaderCardAdapter::BEL)
            {
                if (circular_buffer[0] != PromagReaderCardAdapter::STX)
                {
                    LOG(LogLevel::WARNINGS) << "Drop circular_buffer. Bad command response. STX byte doesn't match.";
                    circular_buffer.clear();
                    return result;
                }

                for (size_t i = 1; i < circular_buffer.size(); ++i)
                {
                    if (circular_buffer[i] == PromagReaderCardAdapter::CR)
                    {
                        result.assign(circular_buffer.begin() + 1, circular_buffer.begin() + i);
                        circular_buffer.erase(circular_buffer.begin(), circular_buffer.begin() + i + 1);
                        break;
                    }
                }
            }
            else
            {
                result.assign(circular_buffer.begin() + 1, circular_buffer.end());
                circular_buffer.erase(circular_buffer.begin(), circular_buffer.end());
            }
        }
        return result;
    }
}