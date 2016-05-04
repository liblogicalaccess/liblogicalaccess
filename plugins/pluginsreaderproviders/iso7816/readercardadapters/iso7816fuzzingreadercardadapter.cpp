/**
* \file iso7816readercardadapter.cpp
* \brief ISO7816 Fuzzing reader/card adapter.
*/

#include "iso7816fuzzingreadercardadapter.hpp"

#include <cstring>

namespace logicalaccess
{
	int ISO7816FuzzingReaderCardAdapter::index = 1;

	ISO7816FuzzingReaderCardAdapter::ISO7816FuzzingReaderCardAdapter() : currentIndex(0)
	{
	}

	std::vector<unsigned char> ISO7816FuzzingReaderCardAdapter::sendCommand(const std::vector<unsigned char>& command, long timeout)
	{
		++currentIndex;
		std::vector<unsigned char> result;
		try
		{
			result = ISO7816ReaderCardAdapter::sendCommand(command, timeout);
		}
		catch (...)
		{
			++index;
			throw;
		}

		if (currentIndex == index)
		{
			if (result.size() > 2)
			{
				std::random_device rnd_device;
				std::mt19937 mersenne_engine(rnd_device());
				std::uniform_int_distribution<int> dist(0x00, 0xff);
				auto gen = std::bind(dist, mersenne_engine);

				std::generate(std::begin(result), std::end(result) - 2, gen);
			}
			++index;
		}

		return result;
	}
}