/**
 * \file admittoreadercardadapter.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Admitto reader/card adapter.
 */

#include "admittoreadercardadapter.hpp"

namespace logicalaccess
{			
	const unsigned char AdmittoReaderCardAdapter::CR = 0x0d;
	const unsigned char AdmittoReaderCardAdapter::LF = 0x0a;

	AdmittoReaderCardAdapter::AdmittoReaderCardAdapter()
		: ReaderCardAdapter()
	{
	}

	AdmittoReaderCardAdapter::~AdmittoReaderCardAdapter()
	{
	}

	bool AdmittoReaderCardAdapter::sendCommandWithoutResponse(const std::vector<unsigned char>& command)
	{
		COM_("Sending command %s command size {%d}...", BufferHelper::getHex(command).c_str(), command.size());
		
		bool ret = false;
		d_lastCommand = command;
		d_lastResult.clear();

		if (command.size() > 0)
		{
			getAdmittoReaderUnit()->connectToReader();

			boost::shared_ptr<SerialPortXml> port = getAdmittoReaderUnit()->getSerialPort();
			port->getSerialPort()->write(command);

			ret = true;
		}

		return ret;
	}

	std::vector<unsigned char> AdmittoReaderCardAdapter::sendCommand(const std::vector<unsigned char>& command, long int timeout)
	{
		COM_("Sending command %s command size {%d} timeout {%d}...", BufferHelper::getHex(command).c_str(), command.size(), timeout);

		std::vector<unsigned char> r;
		if (sendCommandWithoutResponse(command))
		{
			if (!receiveCommand(r, timeout))
			{
				r.clear();
				THROW_EXCEPTION_WITH_LOG(CardException, "No response received");
			}
			else
			{
				COM_("Response received successfully ! Reponse: %s size {%d}", BufferHelper::getHex(r).c_str(), r.size());
			}

			d_lastResult = r;
		}

		return r;
	}

	std::vector<unsigned char> AdmittoReaderCardAdapter::handleCommandBuffer(const std::vector<unsigned char>& cmdbuf)
	{
		COM_("Processing the received command buffer %s command size {%d}...", BufferHelper::getHex(cmdbuf).c_str(), cmdbuf.size());

		EXCEPTION_ASSERT_WITH_LOG(cmdbuf.size() >= 2, std::invalid_argument, "A valid command buffer size must be at least 2 bytes long");
		EXCEPTION_ASSERT_WITH_LOG(cmdbuf[cmdbuf.size()-2] == CR, std::invalid_argument, "The supplied command buffer is not valid (bad first stop byte)");
		EXCEPTION_ASSERT_WITH_LOG(cmdbuf[cmdbuf.size()-1] == LF, std::invalid_argument, "The supplied command buffer is not valid (bad second stop byte)");

		size_t i = 0;
		for (; i < cmdbuf.size()-1; ++i)
		{
			if (cmdbuf[i] == CR && cmdbuf[i+1] == LF)
			{
				break;
			}
		}
		EXCEPTION_ASSERT_WITH_LOG(i < cmdbuf.size(), std::invalid_argument, "The supplied command buffer is not valid (no stop bytes)");
		
		return std::vector<unsigned char>(cmdbuf.begin(), cmdbuf.begin() + i);
	}

	bool AdmittoReaderCardAdapter::receiveCommand(std::vector<unsigned char>& buf, long int timeout)
	{
		COM_("Beginning receiving response... Timeout configured {%d}", timeout);

		std::vector<unsigned char> res;
		boost::shared_ptr<SerialPortXml> port = getAdmittoReaderUnit()->getSerialPort();
		bool timeoutOccured = true;

		boost::posix_time::time_duration ptimeout = boost::posix_time::milliseconds(timeout);
		while (port->getSerialPort()->select(ptimeout))
		{
			std::vector<unsigned char> tmpbuf;
			timeoutOccured = false;

			while (port->getSerialPort()->read(tmpbuf, 256) > 0)
			{
				COM_("Data read on the serial port %s", BufferHelper::getHex(tmpbuf).c_str());

				res.insert(res.end(), tmpbuf.begin(), tmpbuf.end());
				tmpbuf.clear();

				try
				{
					buf = handleCommandBuffer(res);
					return true;
				}
				catch(LibLogicalAccessException& e)
				{
					COM_("Exception {%s}", e.what());
					throw;
				}
				catch (std::invalid_argument& e)
				{
					COM_("Exception {%s}", e.what());
				}
			}
		}

		if (timeoutOccured)
		{
			COM_SIMPLE_("Timeout reached !");
		}
		else
		{
			COM_SIMPLE_("Reponse command not received !");
		}
		return false;
	}	
}
