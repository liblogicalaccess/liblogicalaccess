/**
 * \file stidstrreaderbufferparser.cpp
 * \author Adrien J. <adrien.jund@islog.com>
 * \brief STidSTR buffer parser.
 */

#include "stidstrreaderbufferparser.hpp"

namespace logicalaccess
{		
	std::vector<unsigned char> STidSTRBufferParser::getValidBuffer(boost::circular_buffer<unsigned char>& circular_buffer)
	{
		std::vector<unsigned char> result;

		if (circular_buffer.size() >= 7)
		{
			unsigned short messageSize = (circular_buffer[1] << 8) | circular_buffer[2];
			if (circular_buffer.size() >= static_cast<unsigned short>(messageSize + 7))
			{
				result.assign(circular_buffer.begin(), circular_buffer.begin() + messageSize + 7);
				circular_buffer.erase(circular_buffer.begin(), circular_buffer.begin() + messageSize + 7);
			}
		}
		return result;
	}
}
