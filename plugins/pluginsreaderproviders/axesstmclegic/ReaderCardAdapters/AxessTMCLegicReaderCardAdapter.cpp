/**
 * \file AxessTMCLegicReaderCardAdapter.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief AxessTMCLegic reader/card adapter.
 */

#include "AxessTMCLegicReaderCardAdapter.h"


namespace logicalaccess
{
	AxessTMCLegicReaderCardAdapter::AxessTMCLegicReaderCardAdapter()
		: ReaderCardAdapter()
	{
		
	}

	AxessTMCLegicReaderCardAdapter::~AxessTMCLegicReaderCardAdapter()
	{
		
	}

	bool AxessTMCLegicReaderCardAdapter::sendCommandWithoutResponse(const std::vector<unsigned char>& command)
	{
		bool ret = false;
		d_lastCommand = command;
		d_lastResult.clear();

		if (command.size() > 0)
		{
			getAxessTMCLegicReaderUnit()->connectToReader();

			boost::shared_ptr<SerialPortXml> port = getAxessTMCLegicReaderUnit()->getSerialPort();
			std::vector<unsigned char> cmd = command;
			unsigned char cmdsize = static_cast<unsigned char>(command.size());
			cmd.insert(cmd.begin(), &cmdsize, &cmdsize + 1);
			port->getSerialPort()->write(cmd);

			ret = true;
		}

		return ret;
	}

	std::vector<unsigned char> AxessTMCLegicReaderCardAdapter::sendCommand(const std::vector<unsigned char>& command, long int timeout)
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

	std::vector<unsigned char> AxessTMCLegicReaderCardAdapter::handleCommandBuffer(const std::vector<unsigned char>& cmdbuf)
	{
		EXCEPTION_ASSERT_WITH_LOG(cmdbuf.size() >= 1, std::invalid_argument, "A valid command buffer size must be at least 1 bytes long");
		unsigned char msglength = cmdbuf[0];
		EXCEPTION_ASSERT_WITH_LOG(cmdbuf.size() >= static_cast<unsigned char>(1 + msglength), std::invalid_argument, "The supplied command buffer is not valid (bad messange length)");		
		
		return std::vector<unsigned char>(cmdbuf.begin() + 1, cmdbuf.begin() + 1 + msglength);
	}

	bool AxessTMCLegicReaderCardAdapter::receiveCommand(std::vector<unsigned char>& buf, long int timeout)
	{		
		std::vector<unsigned char> res;
		boost::shared_ptr<SerialPortXml> port = getAxessTMCLegicReaderUnit()->getSerialPort();

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
