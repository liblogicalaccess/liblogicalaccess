/**
 * \file promagreadercardadapter.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Promag reader/card adapter.
 */

#include "promagreadercardadapter.hpp"

namespace logicalaccess
{		
	const unsigned char PromagReaderCardAdapter::STX = 0x02;
	const unsigned char PromagReaderCardAdapter::ESC = 0x1b;
	const unsigned char PromagReaderCardAdapter::BEL = 0x07;
	const unsigned char PromagReaderCardAdapter::CR = 0x0d;
	const unsigned char PromagReaderCardAdapter::LF = 0x0a;

	PromagReaderCardAdapter::PromagReaderCardAdapter()
		: ReaderCardAdapter()
	{
		
	}

	PromagReaderCardAdapter::~PromagReaderCardAdapter()
	{
		
	}

	std::vector<unsigned char> PromagReaderCardAdapter::adaptCommand(const std::vector<unsigned char>& command)
	{
		return command;
	}

	std::vector<unsigned char> PromagReaderCardAdapter::adaptAnswer(const std::vector<unsigned char>& answer)
	{
		std::vector<unsigned char> ret;
		EXCEPTION_ASSERT_WITH_LOG(answer.size() >= 1, std::invalid_argument, "A valid buffer size must be at least 1 bytes long");
		if (answer[0] != ESC && answer[0] != BEL)
		{
			EXCEPTION_ASSERT_WITH_LOG(answer.size() >= 2, std::invalid_argument, "A valid buffer size with data must be at least 2 bytes long");
			EXCEPTION_ASSERT_WITH_LOG(answer[0] == STX, std::invalid_argument, "Bad command response. STX byte doesn't match.");

			size_t i = 1;
			for (; i < answer.size(); ++i)
			{
				if (answer[i] == CR)
				{
					break;
				}
			}
			EXCEPTION_ASSERT_WITH_LOG(i < answer.size(), std::invalid_argument, "The supplied buffer is not valid (no stop bytes)");		

			ret = std::vector<unsigned char>(answer.begin() + 1, answer.begin() + i);
		}
		else
		{
			ret = std::vector<unsigned char>(answer.begin() + 1, answer.end());
		}
		
		return ret;
	}
}
