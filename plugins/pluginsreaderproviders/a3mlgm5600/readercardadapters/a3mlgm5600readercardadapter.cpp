/**
 * \file a3mlgm5600readercardadapter.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief A3MLGM5600 reader/card adapter.
 */

#include "a3mlgm5600readercardadapter.hpp"
#include "logicalaccess/crypto/tomcrypt.h"
#include <ctime>

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

	std::vector<unsigned char> A3MLGM5600ReaderCardAdapter::adaptCommand(const std::vector<unsigned char>& data)
	{
		EXCEPTION_ASSERT_WITH_LOG(data.size() >= 1, std::invalid_argument, "A valid command buffer size must be at least 1 byte long");

		std::vector<unsigned char> command;		
		command.push_back((1 << 7) | (d_seq << 4));	//SEQ
		command.push_back(0);	//DADD
		command.push_back(data[0]);	//CMD
		command.push_back(static_cast<unsigned char>(data.size()));	//DATALENGTH
		command.push_back(0); //TIME
		command.insert(command.end(), data.begin() + 1, data.end());	//DATA
		command.push_back(calcBCC(command));
		command.insert(command.begin(), &STX, &STX + 1);	//STX
		command.push_back(ETX);	//ETX

		d_seq++;
		if (d_seq > 7)
		{
			d_seq = 0;
		}

		return command;
	}

	std::vector<unsigned char> A3MLGM5600ReaderCardAdapter::sendCommand(unsigned char cmd, const std::vector<unsigned char>& data, long int timeout)
	{
		std::vector<unsigned char> command = data;
		command.insert(command.begin(), cmd);

		return ReaderCardAdapter::sendCommand(command, timeout);
	}

	std::vector<unsigned char> A3MLGM5600ReaderCardAdapter::adaptAnswer(const std::vector<unsigned char>& answer)
	{
		EXCEPTION_ASSERT_WITH_LOG(answer.size() >= 7, std::invalid_argument, "A valid buffer size must be at least 7 bytes long");
		EXCEPTION_ASSERT_WITH_LOG(answer[0] == STX, std::invalid_argument, "The supplied buffer is not valid (bad STX byte)");
		
		std::vector<unsigned char> buf = std::vector<unsigned char>(answer.begin() + 1, answer.end());
		unsigned char datalen = buf[2];
		d_command_status = buf[3];		

		EXCEPTION_ASSERT_WITH_LOG(static_cast<unsigned char>((4 + datalen - 1)) <= (buf.size() - 2), std::invalid_argument, "The supplied command buffer is not valid (bad command length)");		
		EXCEPTION_ASSERT_WITH_LOG(buf[4 + datalen] == ETX, std::invalid_argument, "The supplied buffer is not valid (bad ETX byte)");
		buf.resize(buf.size() - 1);
		unsigned char cmdBcc = buf.back();
		buf.pop_back();
		unsigned char bcc = calcBCC(buf);
		buf = std::vector<unsigned char>(buf.begin() + 4, buf.end());
		EXCEPTION_ASSERT_WITH_LOG(cmdBcc == bcc, std::invalid_argument, "The supplied buffer is not valid (bad BCC byte)");					

		return buf;
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
