/**
 * \file gigatmsbufferparser.cpp
 * \author Adrien J. <adrien.jund@islog.com>
 * \brief GIGA-TMS buffer parser.
 */

#include <logicalaccess/logs.hpp>
#include <logicalaccess/plugins/readers/gigatms/readercardadapters/gigatmsbufferparser.hpp>
#include <logicalaccess/plugins/readers/gigatms/readercardadapters/gigatmsreadercardadapter.hpp>

namespace logicalaccess
{
ByteVector GigaTMSBufferParser::getValidBuffer(
    boost::circular_buffer<unsigned char> &circular_buffer)
{
    ByteVector result;

    if (circular_buffer.size() >= 3)
    {
        if (circular_buffer[0] != GigaTMSReaderCardAdapter::STX1 ||
            circular_buffer[1] != GigaTMSReaderCardAdapter::STX2)
        {
            LOG(LogLevel::WARNINGS)
                << "Drop circular_buffer. Bad command response. STX bytes doesn't match.";
            circular_buffer.clear();
            return result;
        }

        if (circular_buffer[2] < 3)
        {
            LOG(LogLevel::WARNINGS) << "Drop circular_buffer. Bad command response. "
                                       "Response length too small.";
            circular_buffer.clear();
            return result;
        }

        if (circular_buffer[2] <= circular_buffer.size() - 4)
        {
            result.assign(circular_buffer.begin(),
                          circular_buffer.begin() + 3 + circular_buffer[2] + 1);
            circular_buffer.erase(circular_buffer.begin(),
                                  circular_buffer.begin() + 3 + circular_buffer[2] + 1);
        }
    }
    return result;
}
}