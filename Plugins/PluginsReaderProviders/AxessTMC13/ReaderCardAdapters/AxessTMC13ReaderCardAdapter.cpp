/**
 * \file AxessTMC13ReaderCardAdapter.cpp
 * \author Maxime CHAMLEY <maxime.chamley@islog.eu>
 * \brief AxessTMC 13Mhz reader/card adapter.
 */

#include "AxessTMC13ReaderCardAdapter.h"

namespace LOGICALACCESS
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

	bool AxessTMC13ReaderCardAdapter::sendCommandWithoutResponse(const std::vector<unsigned char>& command)
	{
		bool ret = false;
		d_lastCommand = command;
		d_lastResult.clear();

		if (command.size() > 0)
		{
			getAxessTMC13ReaderUnit()->connectToReader();

			boost::shared_ptr<SerialPortXml> port = getAxessTMC13ReaderUnit()->getSerialPort();
			port->getSerialPort()->write(command);

			ret = true;
		}

		return ret;
	}

	std::vector<unsigned char> AxessTMC13ReaderCardAdapter::sendCommand(const std::vector<unsigned char>& command, long int timeout)
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

	std::vector<unsigned char> AxessTMC13ReaderCardAdapter::handleCommandBuffer(const std::vector<unsigned char>& cmdbuf)
	{		
		size_t i = 0;
		for (; i < cmdbuf.size(); ++i)
		{
			if (cmdbuf[i] == CR)
			{
				break;
			}
		}
		EXCEPTION_ASSERT_WITH_LOG(i < cmdbuf.size(), std::invalid_argument, "The supplied command buffer is not valid (no stop byte)");		
		
		return std::vector<unsigned char>(cmdbuf.begin(), cmdbuf.begin() + i);
	}

	bool AxessTMC13ReaderCardAdapter::receiveCommand(std::vector<unsigned char>& buf, long int timeout)
	{		
		std::vector<unsigned char> res;
		boost::shared_ptr<SerialPortXml> port = getAxessTMC13ReaderUnit()->getSerialPort();
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
