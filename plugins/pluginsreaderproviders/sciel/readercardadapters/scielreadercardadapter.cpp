/**
 * \file scielreadercardadapter.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief SCIEL reader/card adapter.
 */

#include "scielreadercardadapter.hpp"


namespace logicalaccess
{	
	const unsigned char SCIELReaderCardAdapter::STX = 0x5b;	// [
	const unsigned char SCIELReaderCardAdapter::ETX = 0x5d;	// ]

	SCIELReaderCardAdapter::SCIELReaderCardAdapter()
		: ReaderCardAdapter()
	{
	}

	SCIELReaderCardAdapter::~SCIELReaderCardAdapter()
	{
	}

	std::vector<unsigned char> SCIELReaderCardAdapter::adaptCommand(const std::vector<unsigned char>& command)
	{
		std::vector<unsigned char> cmd;
		// Ignore empty command
		if (command.size()  > 0)
		{
			cmd.push_back(STX);
			cmd.insert(cmd.end(), command.begin(), command.end());
			cmd.push_back(ETX);
		}

		return cmd;
	}

	std::vector<unsigned char> SCIELReaderCardAdapter::adaptAnswer(const std::vector<unsigned char>& answer)
	{
		std::vector<unsigned char> buf;
		if (d_trashedData.size() > 0)
		{
			LOG(LogLevel::COMS) << "Adding existing trashed data: " << BufferHelper::getHex(d_trashedData);
			buf = d_trashedData;
		}
		buf.insert(buf.end(), answer.begin(), answer.end());

		// Remove CR/LF (some response have it ?!)
		while (buf.size() >= 2 && buf[0] == 0x0d)
		{
			int cuti = 1;
			if (buf[1] == 0x0a)
			{
				cuti++;
			}

			buf = std::vector<unsigned char>(buf.begin() + cuti, buf.end());
		}

		EXCEPTION_ASSERT_WITH_LOG(buf.size() >= 2, LibLogicalAccessException, "A valid buffer size must be at least 2 bytes long");
		EXCEPTION_ASSERT_WITH_LOG(buf[0] == STX, LibLogicalAccessException, "The supplied buffer is not valid");
		
		std::vector<unsigned char> data;

		int i = 1;
		while(buf.size() > 0 && buf[i] != ETX)
		{
			unsigned char c = buf[i++];
			data.push_back(c);
		}
		buf = std::vector<unsigned char>(buf.begin() + i, buf.end());

		EXCEPTION_ASSERT_WITH_LOG(buf.size() > 0 && buf[0] == ETX, LibLogicalAccessException, "Missing end of message");

		buf = std::vector<unsigned char>(buf.begin() + 1, buf.end());
		d_trashedData = buf;

		LOG(LogLevel::COMS) << "Returning processed data " << BufferHelper::getHex(data) << "...";
		LOG(LogLevel::COMS) << "	-> Actual trashed data %s..." << BufferHelper::getHex(d_trashedData) << "...";
		return data;
	}

	std::list<std::vector<unsigned char> > SCIELReaderCardAdapter::receiveTagsListCommand(const std::vector<unsigned char>& command, long int timeout)
	{
		LOG(LogLevel::COMS) << "Beginning receiving tag(s) identifiers whole list... Timeout configured {" << timeout << "}";

		std::list<std::vector<unsigned char>> tagsList;

		std::vector<unsigned char> tag = sendCommand(command, timeout);
		bool done = false;
		while (!done && tag.size() > 0)
		{	
			// When whole list is sent, the reader send [5b nb_tags id_reader 5d]
			if (tag.size() == 2)
			{
				LOG(LogLevel::COMS) << "Whole list has been received successfully!";
				done = true;
			}
			else
			{
				tagsList.push_back(tag);
			}

			// Next time are optional
			try
			{
				tag = sendCommand(std::vector<unsigned char>(), timeout);
			}
			catch(std::exception&)
			{
			}
		}

		return tagsList;
	}
}
