/**
 * \file ElatecReaderCardAdapter.cpp
 * \author Maxime CHAMLEY <maxime.chamley@islog.eu>
 * \brief Elatec reader/card adapter.
 */

#include "ElatecReaderCardAdapter.h"

namespace LOGICALACCESS
{		
	ElatecReaderCardAdapter::ElatecReaderCardAdapter()
		: ReaderCardAdapter()
	{
		
	}

	ElatecReaderCardAdapter::~ElatecReaderCardAdapter()
	{
		
	}

	unsigned char ElatecReaderCardAdapter::calcChecksum(const std::vector<unsigned char>& buf) const
	{
		unsigned char checksum = 0x00;

		for (size_t i = 0; i < buf.size(); ++i)
		{
			checksum = checksum ^ buf[i];
		}

		return checksum;
	}

	bool ElatecReaderCardAdapter::sendCommandWithoutResponse(unsigned char cmdcode, const std::vector<unsigned char>& command)
	{
		unsigned char tmpcmd[3];
		std::vector<unsigned char> cmd = command;
		tmpcmd[0] = cmdcode;
		tmpcmd[1] = 0xFF;	// all readers
		tmpcmd[2] = static_cast<unsigned char>(cmd.size() + 2);
		cmd.insert(cmd.begin(), tmpcmd, tmpcmd + 3);
		cmd.push_back(calcChecksum(cmd));
		d_lastCommand = cmd;
		d_last_cmdcode = cmdcode;
		d_lastResult.clear();

		getElatecReaderUnit()->connectToReader();

		boost::shared_ptr<SerialPortXml> port = getElatecReaderUnit()->getSerialPort();
		port->getSerialPort()->write(cmd);		

		return true;
	}

	std::vector<unsigned char> ElatecReaderCardAdapter::sendCommand(const std::vector<unsigned char>& command, long int timeout)
	{
		EXCEPTION_ASSERT_WITH_LOG(command.size() >= 1, std::invalid_argument, "A valid command buffer should at least contains command code");

		return sendCommand(command[0], std::vector<unsigned char>(command.begin() + 1, command.end()), timeout);
	}

	std::vector<unsigned char> ElatecReaderCardAdapter::sendCommand(unsigned char cmdcode, const std::vector<unsigned char>& command, long int timeout)
	{
		std::vector<unsigned char> r;
		if (sendCommandWithoutResponse(cmdcode, command))
		{
			if (!receiveCommand(r, timeout))
			{
				THROW_EXCEPTION_WITH_LOG(CardException, "No response received");
			}

			d_lastResult = r;
		}

		return r;
	}

	std::vector<unsigned char> ElatecReaderCardAdapter::handleCommandBuffer(const std::vector<unsigned char>& cmdbuf)
	{
		std::vector<unsigned char> ret;
		EXCEPTION_ASSERT_WITH_LOG(cmdbuf.size() >= 5, std::invalid_argument, "A valid command buffer size must be at least 5 bytes long");
		
		unsigned char buflength = cmdbuf[0];
		EXCEPTION_ASSERT_WITH_LOG(cmdbuf.size() >= buflength, std::invalid_argument, "The buffer is too short according to the excepted buffer length");

		EXCEPTION_ASSERT_WITH_LOG(cmdbuf[2] == d_last_cmdcode, std::invalid_argument, "The request type doesn't match the command sent by the host");

		unsigned char status = cmdbuf[3];
		// Don't throw exception for "no card" when getting card list
		if (d_last_cmdcode != 0x11 && (status != 0x01))
		{
			EXCEPTION_ASSERT_WITH_LOG(status == 0x00, std::invalid_argument, "Elatec error");
		}

		unsigned char checksum = cmdbuf[buflength-1];
		EXCEPTION_ASSERT_WITH_LOG(calcChecksum(std::vector<unsigned char>(cmdbuf.begin(), cmdbuf.begin() + buflength - 1)) == checksum, std::invalid_argument, "Wrong checksum");

		ret = std::vector<unsigned char>(cmdbuf.begin() + 4, cmdbuf.begin() + 4 + buflength - 5);
		
		return ret;
	}

	bool ElatecReaderCardAdapter::receiveCommand(std::vector<unsigned char>& buf, long int timeout)
	{		
		std::vector<unsigned char> res;
		boost::shared_ptr<SerialPortXml> port = getElatecReaderUnit()->getSerialPort();

		boost::posix_time::time_duration ptimeout = boost::posix_time::milliseconds(timeout);
		while (port->getSerialPort()->select(ptimeout))
		{
			std::vector<unsigned char> tmpbuf;

			while (port->getSerialPort()->read(tmpbuf, 256) > 0)
			{
				res.insert(res.end(), tmpbuf.begin(), tmpbuf.end());

				try
				{
					buf = handleCommandBuffer(res);

					return true;
				}
				catch (std::invalid_argument&)
				{
				}
			}
		}

		return false;
	}	
}
