/**
 * \file scielbufferparser.cpp
 * \author Adrien J. <adrien.jund@islog.com>
 * \brief Sciel buffer parser.
 */

#include "scielbufferparser.hpp"
#include "scielreadercardadapter.hpp"

namespace logicalaccess
{		
	std::vector<unsigned char> ScielBufferParser::getValidBuffer(boost::circular_buffer<unsigned char>& circular_buffer)
	{
		std::vector<unsigned char> result;

		if (circular_buffer.size() >= 2)
		{
			// Remove CR/LF (some response have it ?!)
			while (circular_buffer.size() >= 2 && circular_buffer[0] == 0x0d)
			{
				int cuti = 1;
				if (circular_buffer[1] == 0x0a)
				{
					cuti++;
				}

				circular_buffer = boost::circular_buffer<unsigned char>(circular_buffer.begin() + cuti, circular_buffer.end());
			}

			if (circular_buffer.size() >= 2)
			{
				if (circular_buffer[0] != SCIELReaderCardAdapter::STX)
				{
					LOG(LogLevel::WARNINGS) << "Drop circular_buffer, STX not found";
					return result;
				}

				for (size_t i = 1; i < circular_buffer.size(); ++i)
				{
					if (circular_buffer[i] == SCIELReaderCardAdapter::ETX)
					{
						result.assign(circular_buffer.begin(), circular_buffer.begin() + i + 1);
						circular_buffer.erase(circular_buffer.begin(), circular_buffer.begin() + i + 1);
						break;
					}
				}
			}
		}
		return result;
	}
}
