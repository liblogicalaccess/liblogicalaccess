/**
 * \file scielreadercardadapter.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief SCIEL reader/card adapter.
 */

#include "scielreadercardadapter.hpp"
#include "scieldatatransport.hpp"


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
		EXCEPTION_ASSERT_WITH_LOG(answer.size() >= 2, LibLogicalAccessException, "A valid buffer size must be at least 2 bytes long");
		EXCEPTION_ASSERT_WITH_LOG(answer[0] == STX, LibLogicalAccessException, "The supplied buffer is not valid");
		EXCEPTION_ASSERT_WITH_LOG(answer[answer.size() - 1] == ETX, LibLogicalAccessException, "Missing end of message");

		std::vector<unsigned char> data(answer.begin() + 1, answer.end() - 1);

		LOG(LogLevel::COMS) << "Returning processed data " << BufferHelper::getHex(data) << "...";
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
            if (tag.size() == 2)
            {
                LOG(LogLevel::COMS) << "Whole list has been received successfully!";
                done = true;
            }
            else
            {
			    tagsList.push_back(tag);

                boost::shared_ptr<ScielDataTransport> sdt = boost::dynamic_pointer_cast<ScielDataTransport>(getDataTransport());
                if (sdt)
                {
                    std::vector<unsigned char> tagbuf = sdt->checkValideBufferAvailable();
                    if (tagbuf.size() > 0)
                    {
                        tag = adaptAnswer(tagbuf);
                    }
                    else
                    {
                        tag = std::vector<unsigned char>();
                    }
                }
                else
                {
                    tag = std::vector<unsigned char>();
                }
            }
		}

		return tagsList;
	}
}
