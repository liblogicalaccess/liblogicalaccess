/**
 * \file axesstmc13readercardadapter.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief AxessTMC 13Mhz reader/card adapter.
 */

#include "axesstmc13readercardadapter.hpp"

namespace logicalaccess
{		
	const unsigned char AxessTMC13ReaderCardAdapter::START0 = 'N';
	const unsigned char AxessTMC13ReaderCardAdapter::START1 = 'F';
	const unsigned char AxessTMC13ReaderCardAdapter::CR = 0x0d;

	AxessTMC13ReaderCardAdapter::AxessTMC13ReaderCardAdapter()
		: ReaderCardAdapter()
	{
		
	}

	AxessTMC13ReaderCardAdapter::~AxessTMC13ReaderCardAdapter()
	{
		
	}

	std::vector<unsigned char> AxessTMC13ReaderCardAdapter::adaptCommand(const std::vector<unsigned char>& command)
	{
		return command;
	}

	std::vector<unsigned char> AxessTMC13ReaderCardAdapter::adaptAnswer(const std::vector<unsigned char>& answer)
	{		
		size_t i = 0;
		for (; i < answer.size(); ++i)
		{
			if (answer[i] == CR)
			{
				break;
			}
		}
		EXCEPTION_ASSERT_WITH_LOG(i < answer.size(), std::invalid_argument, "The supplied buffer is not valid (no stop byte)");		
		
		return std::vector<unsigned char>(answer.begin(), answer.begin() + i);
	}
}
