/**
 * \file a3mlgm5600readercardadapter.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief A3MLGM5600 reader/card adapter.
 */

#include "a3mlgm5600readercardadapter.hpp"
#include "logicalaccess/crypto/tomcrypt.h"
#include <ctime>

#include <boost/foreach.hpp>
#include <boost/optional.hpp>
#include <boost/array.hpp>

namespace logicalaccess
{	
	A3MLGM5600ReaderCardAdapter::A3MLGM5600ReaderCardAdapter()
		: ReaderCardAdapter()
	{
		d_seq = 0;
		d_command_status = 0;
	}

	A3MLGM5600ReaderCardAdapter::~A3MLGM5600ReaderCardAdapter()
	{
		
	}

	unsigned char A3MLGM5600ReaderCardAdapter::calcBCC(const std::vector<unsigned char>& data)
	{
		unsigned char bcc = 0x00;

		for (unsigned int i = 0; i < data.size(); ++i)
		{
			bcc ^= data[i];
		}

		return bcc;
	}

	bool A3MLGM5600ReaderCardAdapter::sendCommandWithoutResponse(unsigned char cmd, const std::vector<unsigned char>& data)
	{
		bool ret = false;
		std::vector<unsigned char> command;		
		command.push_back((1 << 7) | (d_seq << 4));	//SEQ
		command.push_back(0);	//DADD
		command.push_back(cmd);	//CMD
		command.push_back(static_cast<unsigned char>(1 + data.size()));	//DATALENGTH
		command.push_back(0); //TIME
		command.insert(command.end(), data.begin(), data.end());	//DATA
		command.push_back(calcBCC(command));
		command.insert(command.begin(), &STX, &STX + 1);	//STX
		command.push_back(ETX);	//ETX
		d_lastCommand = command;
		d_lastResult.clear();

		if (command.size() > 0)
		{
			getA3MLGM5600ReaderUnit()->connectToReader();

			boost::shared_ptr<boost::asio::ip::udp::socket> socket = getA3MLGM5600ReaderUnit()->getSocket();
			socket->send(boost::asio::buffer(command));

			d_seq++;
			if (d_seq > 7)
			{
				d_seq = 0;
			}

			ret = true;
		}

		return ret;
	}

	std::vector<unsigned char> A3MLGM5600ReaderCardAdapter::sendCommand(const std::vector<unsigned char>& command, long int timeout)
	{		
		return sendCommand(command[0], std::vector<unsigned char>(command.begin(), command.end() - 1), timeout);
	}

	std::vector<unsigned char> A3MLGM5600ReaderCardAdapter::sendCommand(unsigned char cmd, const std::vector<unsigned char>& data, long int timeout)
	{
		std::vector<unsigned char> r;
		if (sendCommandWithoutResponse(cmd, data))
		{
			if (!receiveCommand(r, timeout))
			{
				THROW_EXCEPTION_WITH_LOG(CardException, "No response received");
			}

			d_lastResult = r;
		}

		return r;
	}

	std::vector<unsigned char> A3MLGM5600ReaderCardAdapter::handleCommandBuffer(const std::vector<unsigned char>& cmdbuf)
	{
		EXCEPTION_ASSERT_WITH_LOG(cmdbuf.size() >= 7, std::invalid_argument, "A valid command buffer size must be at least 7 bytes long");
		EXCEPTION_ASSERT_WITH_LOG(cmdbuf[0] == STX, std::invalid_argument, "The supplied command buffer is not valid (bad STX byte)");
		
		std::vector<unsigned char> buf = std::vector<unsigned char>(cmdbuf.begin() + 1, cmdbuf.end());
		unsigned char datalen = buf[2];
		d_command_status = buf[3];		

		EXCEPTION_ASSERT_WITH_LOG(static_cast<unsigned char>((4 + datalen - 1)) <= (buf.size() - 2), std::invalid_argument, "The supplied command buffer is not valid (bad command length)");		
		EXCEPTION_ASSERT_WITH_LOG(buf[4 + datalen] == ETX, std::invalid_argument, "The supplied command buffer is not valid (bad ETX byte)");
		buf.resize(buf.size() - 1);
		unsigned char cmdBcc = buf.back();
		buf.pop_back();
		unsigned char bcc = calcBCC(buf);
		buf = std::vector<unsigned char>(buf.begin() + 4, buf.end());
		EXCEPTION_ASSERT_WITH_LOG(cmdBcc == bcc, std::invalid_argument, "The supplied command buffer is not valid (bad BCC byte)");					

		return buf;
	}

	void set_result(boost::optional<boost::system::error_code>* a, boost::system::error_code b) 
	{ 
		a->reset(b); 
	}

	bool A3MLGM5600ReaderCardAdapter::receiveCommand(std::vector<unsigned char>& buf, long int timeout)
	{
		boost::shared_ptr<boost::asio::ip::udp::socket> socket = getA3MLGM5600ReaderUnit()->getSocket();
		// provisional size
		std::vector<unsigned char> bufrcv(1024);
		std::vector<unsigned char> msg(1024);

		struct timeval tv;		
		tv.tv_usec = 0;
		tv.tv_sec = static_cast<int>(timeout / 1000);

		try
		{
			boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();
			boost::posix_time::ptime nowend = now + boost::posix_time::milliseconds(timeout);

			boost::array<char, 128> recv_buf;
			boost::asio::ip::udp::endpoint sender_endpoint;
			// TODO: Need to set up a timeout here !
			size_t len = socket->receive_from(boost::asio::buffer(recv_buf), sender_endpoint);

			try
			{
				buf = handleCommandBuffer(std::vector<unsigned char>(recv_buf.begin(), recv_buf.begin() + len));

				return true;
			}
			catch (std::invalid_argument&)
			{
			}
		}
		catch(...)
		{
		}

		return false;
	}

	std::vector<unsigned char> A3MLGM5600ReaderCardAdapter::requestA()
	{
		std::vector<unsigned char> buf;
		buf.push_back(0x52);	// 0x26: Request Idle, 0x52: Request all (wake up all)

		return sendCommand(0x30, buf);
	}

	std::vector<unsigned char> A3MLGM5600ReaderCardAdapter::requestATS()
	{
		/* TODO: Something ? */
		return std::vector<unsigned char>();
	}

	void A3MLGM5600ReaderCardAdapter::haltA()
	{
		sendCommand(0x33, std::vector<unsigned char>());
	}

	std::vector<unsigned char> A3MLGM5600ReaderCardAdapter::anticollisionA()
	{
		std::vector<unsigned char> fulluid;
		std::vector<unsigned char> partuid;
		std::vector<unsigned char> buf;
		
		// Cascade level 1
		buf = sendCommand(0x31, std::vector<unsigned char>());
		if (buf.size() == 5)
		{
			partuid = std::vector<unsigned char>(buf.begin(), buf.begin() + 4);
			fulluid.insert(fulluid.end(), partuid.begin(), partuid.end());
			sendCommand(0x32, partuid);
			if (d_command_status == 0x46)
			{
				// Cascade level 2
				buf = sendCommand(0x38, std::vector<unsigned char>());
				if (buf.size() == 5)
				{
					partuid = std::vector<unsigned char>(buf.begin(), buf.begin() + 4);
					fulluid.insert(fulluid.end(), partuid.begin(), partuid.end());
					sendCommand(0x39, partuid);
					if (d_command_status == 0x46)
					{
						// Cascade level 3
						buf = sendCommand(0x3a, std::vector<unsigned char>());
						if (buf.size() == 5)
						{
							partuid = std::vector<unsigned char>(buf.begin(), buf.begin() + 4);
							fulluid.insert(fulluid.end(), partuid.begin(), partuid.end());
							sendCommand(0x3b, partuid);
						}
					}
				}
			}
		}

		return fulluid;
	}

	std::vector<unsigned char> A3MLGM5600ReaderCardAdapter::requestB(unsigned char /*afi*/)
	{
		/* TODO: Not implemented yet */
		return std::vector<unsigned char>();
	}

	void A3MLGM5600ReaderCardAdapter::haltB()
	{
		/* TODO: Not implemented yet */
	}

	void A3MLGM5600ReaderCardAdapter::attrib()
	{
		/* TODO: Not implemented yet */
	}

	std::vector<unsigned char> A3MLGM5600ReaderCardAdapter::anticollisionB(unsigned char /*afi*/)
	{
		/* TODO: Not implemented yet */
		return std::vector<unsigned char>();
	}
}
