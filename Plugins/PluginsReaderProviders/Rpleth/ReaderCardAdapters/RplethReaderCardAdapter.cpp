/**
 * \file RplethReaderCardAdapter.cpp
 * \author Maxime CHAMLEY <maxime.chamley@islog.eu>
 * \brief Rpleth reader/card adapter.
 */

#include "RplethReaderCardAdapter.h"

namespace LOGICALACCESS
{		
	RplethReaderCardAdapter::RplethReaderCardAdapter()
		: ReaderCardAdapter()
	{
		
	}

	RplethReaderCardAdapter::~RplethReaderCardAdapter()
	{
		
	}

	std::vector<unsigned char> RplethReaderCardAdapter::sendCommand(const std::vector<unsigned char>& command, long int timeout)
	{
		std::vector<unsigned char> res;
		std::vector<unsigned char> cmd = command;
		cmd.push_back (static_cast<unsigned char>(calcChecksum(command)));
		if (cmd.size() > 0)
		{
			getRplethReaderUnit()->connectToReader();

			boost::shared_ptr<boost::asio::ip::tcp::socket> socket = getRplethReaderUnit()->getSocket();
			socket->send(boost::asio::buffer(cmd));
			res = receiveAnwser(command, timeout);
		}
		return res;
	}

	std::vector<unsigned char> RplethReaderCardAdapter::receiveAnwser(const std::vector<unsigned char>& command, long int timeout)
	{
		std::vector<unsigned char> res;
		boost::shared_ptr<boost::asio::ip::tcp::socket> socket = getRplethReaderUnit()->getSocket();
		long int currentWait = 0;
		while (socket->available() == 0 && (timeout == 0 || currentWait < timeout))
		{  
	#ifdef _WINDOWS
			Sleep(250);
	#elif defined(LINUX)
			usleep(250000);
	#endif
			currentWait += 250;
		}
		try
		{
			socket->receive (boost::asio::buffer(res));
			if (res.size() > 0)
				res = handleAnswerBuffer (command, res);
		}
		catch (std::invalid_argument&)
		{
		}
		return res;
	}

	unsigned char RplethReaderCardAdapter::calcChecksum(const std::vector<unsigned char>& data)
	{
		unsigned char bcc = 0x00;

		for (unsigned int i = 0; i < data.size(); ++i)
		{
			bcc ^= data[i];
		}

		return bcc;
	}

	std::vector<unsigned char> RplethReaderCardAdapter::handleAnswerBuffer(const std::vector<unsigned char>& cmdbuf, std::vector<unsigned char>& ansbuf)
	{
		EXCEPTION_ASSERT_WITH_LOG(ansbuf.size() > 4, std::invalid_argument, "A valid answer buffer size must be at least 4 bytes long");
		EXCEPTION_ASSERT_WITH_LOG(ansbuf[0] != 0x01, std::invalid_argument, "The supplied answer buffer get the stat : Command failure");
		EXCEPTION_ASSERT_WITH_LOG(ansbuf[0] != 0x02, std::invalid_argument, "The supplied answer buffer get the stat : Bad checksum in command");
		EXCEPTION_ASSERT_WITH_LOG(ansbuf[0] != 0x03, std::invalid_argument, "The supplied answer buffer get the stat : Timeout");
		EXCEPTION_ASSERT_WITH_LOG(ansbuf[0] != 0x04, std::invalid_argument, "The supplied answer buffer get the stat : Bad size of command");
		EXCEPTION_ASSERT_WITH_LOG(ansbuf[0] != 0x05, std::invalid_argument, "The supplied answer buffer get the stat : Bad device in command");
		EXCEPTION_ASSERT_WITH_LOG(ansbuf[1] == cmdbuf [0] && ansbuf[2] == cmdbuf [1], std::invalid_argument, "The supplied answer buffer is not corresponding with command send");

		std::vector<unsigned char> res;
		unsigned char checksum_receive = ansbuf[ansbuf.size()-1];
		ansbuf.pop_back();
		EXCEPTION_ASSERT_WITH_LOG(calcChecksum(ansbuf) == checksum_receive, std::invalid_argument, "The supplied answer buffer get the stat : Bad checksum in answer");
		if (ansbuf.size() > 4)
			res = std::vector<unsigned char> (ansbuf.begin()+4, ansbuf.end());
		return res;
	}
}
