/**
 * \file gunneboreadercardadapter.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Gunnebo reader/card adapter.
 */

#include "gunneboreadercardadapter.hpp"

namespace logicalaccess
{			
	const unsigned char GunneboReaderCardAdapter::STX = 0x02;
	const unsigned char GunneboReaderCardAdapter::ETX = 0x03;

	GunneboReaderCardAdapter::GunneboReaderCardAdapter()
		: ReaderCardAdapter()
	{
	}

	GunneboReaderCardAdapter::~GunneboReaderCardAdapter()
	{
	}

	std::vector<unsigned char> GunneboReaderCardAdapter::adaptCommand(const std::vector<unsigned char>& command)
	{
		return command;
	}

	std::vector<unsigned char> GunneboReaderCardAdapter::adaptAnswer(const std::vector<unsigned char>& answer)
	{
		LOG(LogLevel::COMS) << , "Processing the received command buffer %s command size {%d}...", BufferHelper::getHex(answer).c_str(), answer.size());

		EXCEPTION_ASSERT_WITH_LOG(answer.size() >= 3, std::invalid_argument, "A valid command buffer size must be at least 3 bytes long");
		EXCEPTION_ASSERT_WITH_LOG(answer[0] == STX, std::invalid_argument, "The supplied command buffer is not valid (bad STX)");
		EXCEPTION_ASSERT_WITH_LOG(answer[answer.size()-2] == ETX, std::invalid_argument, "The supplied command buffer is not valid (bad ETX)");
		
		// Gunnebo ID is like that: [STX - 1 byte][id - x bytes][ETX - 1 byte][checksum - 1 byte]
		// We don't calculate the checksum ! Who cares ?!? :)

		return std::vector<unsigned char>(answer.begin() + 1, answer.end() - 2);
	}
}
