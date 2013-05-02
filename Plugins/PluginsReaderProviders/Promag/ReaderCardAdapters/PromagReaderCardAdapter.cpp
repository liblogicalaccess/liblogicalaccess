/**
 * \file PromagReaderCardAdapter.cpp
 * \author Maxime CHAMLEY <maxime.chamley@islog.eu>
 * \brief Promag reader/card adapter.
 */

#include "PromagReaderCardAdapter.h"

namespace LOGICALACCESS
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

	bool PromagReaderCardAdapter::sendCommandWithoutResponse(const std::vector<unsigned char>& command)
	{
		bool ret = false;
		d_lastCommand = command;
		d_lastResult.clear();

		if (command.size() > 0)
		{
			getPromagReaderUnit()->connectToReader();

			boost::shared_ptr<SerialPortXml> port = getPromagReaderUnit()->getSerialPort();
			port->getSerialPort()->write(command);

			ret = true;
		}

		return ret;
	}

	std::vector<unsigned char> PromagReaderCardAdapter::sendCommand(const std::vector<unsigned char>& command, long int timeout)
	{
		std::vector<unsigned char> r;
		if (sendCommandWithoutResponse(command))
		{
			if (!receiveCommand(r, timeout))
			{
				THROW_EXCEPTION_WITH_LOG(CardException, "No response received");
			}

			d_lastResult = r;
		}

		return r;
	}

	std::vector<unsigned char> PromagReaderCardAdapter::handleCommandBuffer(const std::vector<unsigned char>& cmdbuf)
	{
		std::vector<unsigned char> ret;
		EXCEPTION_ASSERT_WITH_LOG(cmdbuf.size() >= 1, std::invalid_argument, "A valid command buffer size must be at least 1 bytes long");
		if (cmdbuf[0] != ESC && cmdbuf[0] != BEL)
		{
			EXCEPTION_ASSERT_WITH_LOG(cmdbuf.size() >= 2, std::invalid_argument, "A valid command buffer size with data must be at least 2 bytes long");
			EXCEPTION_ASSERT_WITH_LOG(cmdbuf[0] == STX, std::invalid_argument, "Bad command response. STX byte doesn't match.");

			size_t i = 1;
			for (; i < cmdbuf.size(); ++i)
			{
				if (cmdbuf[i] == CR)
				{
					break;
				}
			}
			EXCEPTION_ASSERT_WITH_LOG(i < cmdbuf.size(), std::invalid_argument, "The supplied command buffer is not valid (no stop bytes)");		

			ret = std::vector<unsigned char>(cmdbuf.begin() + 1, cmdbuf.begin() + i);
		}
		else
		{
			ret = std::vector<unsigned char>(cmdbuf.begin() + 1, cmdbuf.end());
		}
		
		return ret;
	}

	bool PromagReaderCardAdapter::receiveCommand(std::vector<unsigned char>& buf, long int timeout)
	{		
		std::vector<unsigned char> res;
		boost::shared_ptr<SerialPortXml> port = getPromagReaderUnit()->getSerialPort();

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
				catch (std::exception&)
				{
				}
			}
		}

		return false;
	}	
}
