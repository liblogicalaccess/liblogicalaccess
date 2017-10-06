/**
 * \file osdpbufferparser.cpp
 * \author Adrien J. <adrien.jund@islog.com>
 * \brief OSDP buffer parser.
 */

#include <logicalaccess/logs.hpp>
#include "osdpbufferparser.hpp"

namespace logicalaccess
{
    std::vector<unsigned char> OSDPBufferParser::getValidBuffer(boost::circular_buffer<unsigned char>& circular_buffer)
    {
	std::vector<unsigned char> result;

	int removeCount = 0;
	for (auto it = circular_buffer.begin(); it != circular_buffer.end() && (*it != 0x53); ++it)
	{
	    ++removeCount;
	}
	if (removeCount != 0)
	    LOG(LogLevel::DEBUGS) << "Remove noise length: " << removeCount;
	circular_buffer.erase(circular_buffer.begin(), circular_buffer.begin() + removeCount);
	//we expect that is start by 0x53 - everything else is noise

	if (circular_buffer.size() >= 6)
	{
	    short packetLength = ((circular_buffer[2 + 1] << 8) + circular_buffer[2]);
	    LOG(LogLevel::DEBUGS) << "packetLength requested: " << packetLength;
	    if (static_cast<short>(circular_buffer.size()) >= packetLength)
	    {
		result.assign(circular_buffer.begin(), circular_buffer.begin() + packetLength);
		circular_buffer.erase(circular_buffer.begin(), circular_buffer.begin() + packetLength);
	    }
	}
	return result;
    }
}
